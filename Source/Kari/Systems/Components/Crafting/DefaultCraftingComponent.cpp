

#include "DefaultCraftingComponent.h"


#include "Characters/Playables/DefaultPlayer.h"
#include "Kismet/GameplayStatics.h"

#include "Systems/Components/InteractableComponent.h"
#include "General/DefaultGameInstance.h"
#include "Systems/Components/StoryComponent.h"
#include "UI/DefaultCraftingWidget.h"
#include "UI/Dialogues/DialogueWidget.h"
#include "Systems/Components/CharacterComponent.h"

UDefaultCraftingComponent::UDefaultCraftingComponent() : UTableComponent("shops", "name", "parentId")
{	
}

UDefaultCraftingComponent::UDefaultCraftingComponent(const FString& name, const FString& dropdownColumn,
	const FString& linkedColumn, uint32 linkedId) : UTableComponent(name, dropdownColumn, linkedColumn, linkedId)
{
}

void UDefaultCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

    UInteractableComponent* Interactable = Cast<UInteractableComponent>(GetOwner()->GetComponentByClass(UInteractableComponent::StaticClass()));
    ensure(Interactable);

    if (Interactable)
    {
        Interactable->Action = "Buy from ";
    }

	DefaultGameInstance = Cast<UDefaultGameInstance>(GetOwner()->GetGameInstance());
}

void UDefaultCraftingComponent::ShowMenu_Implementation()
{
	if (Shown)
		return;

	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if(Player)
		Player->SetInteractionCharacterID(CharacterComponent->GetTableID(), true);

	const FStringClassReference WidgetReference = FStringClassReference(TEXT("/Game/UI/Trade/WBP_Trade.WBP_Trade_C"));
	UClass* WidgetClass = WidgetReference.TryLoadClass<UDefaultCraftingWidget>();

	if (WidgetClass)
	{
		TradeWidget = CreateWidget<UDefaultCraftingWidget>(
			UGameplayStatics::GetPlayerController(GetWorld(), 0),
			WidgetClass
		);

		if (TradeWidget)
		{
			TradeWidget->SetTradeComponent(this);
			TradeWidget->AddToViewport();

			Shown = true;
		}
	}
}

void UDefaultCraftingComponent::HideMenu_Implementation()
{
	Shown = false;

	if (TradeWidget)
	{		
		TradeWidget->RemoveFromParent();
		TradeWidget = nullptr;

		ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		if(Player)
			Player->SetInteractionCharacterID(-INT_MAX, true);
		
		if(NextDialogueId != UINT_MAX)
		{
			auto* StoryComponent = CharacterComponent->GetStoryComponent();
			StoryComponent->ShowDialogue(NextDialogueId);
		}
	}
}

TArray<FCraftSlot> UDefaultCraftingComponent::PopulateCraftableItems_Implementation()
{
	TArray<FCraftSlot> Slots;

	// Retrieve the craftables of the shop owner
	TArray<TPair<uint32, const FTableRow*>> Links = FindLinks("shopCraftables", "parentId", ID);

	for (auto& Link : Links)
	{
		FCraftSlot Slot;

		uint32 CraftableId = Link.Key;

		// Assign right field to the craftable items
		for (auto& Field : Link.Value->Fields)
		{
			if (Field.Key.ColumnName == "childId")
			{
				double d;
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

				const FTableRow& Row = GetRow("items", d);
				Slot.Craftable = FItem::ConvertRow(Row, d);	
			}
			else if (Field.Key.ColumnName == "shopRevisionId")
			{
				// Todo: Check current revision ID
			}
			else if (Field.Key.ColumnName == "value")
			{
				double d;
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

				Slot.Value = d;
			}
		}
				
		// retrieve the crafting conditions based on the item found.
		TArray<TPair<uint32, const FTableRow*>> ConditionLinks = FindLinks("shopCraftConditions", "parentId", CraftableId);

		// Assign fields to the craftable requirements.
		for (auto& Row : ConditionLinks)
		{
			FCraftRequirement Requirement;
			for (auto& Field : Row.Value->Fields)
			{
				if (Field.Key.ColumnName == "childId")
				{
					double d;
					memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

					const FTableRow& R = GetRow("items", d);
					Requirement.Item = FItem::ConvertRow(R, d);
				}
				else if (Field.Key.ColumnName == "shopRevisionId")
				{
					// Todo: Check current revision ID
				}
				else if (Field.Key.ColumnName == "amount")
				{
					double d;
					memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

					Requirement.Amount = d;
				}
			}
			Slot.Requirements.Add(Requirement);
		}
		
		Slots.Add(Slot);
	}

	return Slots;
}

TArray<FTradeSlot> UDefaultCraftingComponent::PopulateItems() const
{
	TArray<FTradeSlot> Slots;
	TArray<TPair<uint32, const FTableRow*>> Links = FindLinks("shopPrices", "parentId", ID);

	for (auto& Link : Links)
	{
		FTradeSlot Slot;

		for (auto& Field : Link.Value->Fields)
		{
			if (Field.Key.ColumnName == "childId")
			{
				double d;
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

				const FTableRow& Row = GetRow("items", d);
				Slot.Item = FItem::ConvertRow(Row, d);
			}
			else if (Field.Key.ColumnName == "maxQuantity")
			{
				double d;
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

				Slot.Count = d;
			}
			else if (Field.Key.ColumnName == "shopRevisionId")
			{
				// Todo: Check current revision ID
			}
			else if (Field.Key.ColumnName == "value")
			{
				double d;
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

				Slot.Value = d;
			}
		}

		Slots.Add(Slot);
	}

	return Slots;
}


/*TArray<FCraftSlot> UDefaultCraftingComponent::PopulateCraftables() const
{
	TArray<FCraftSlot> Slots;
	TArray<TPair<uint32, const FTableRow*>> Links = FindLinks("", "parentId", ID);

	for (auto& Link : Links)
	{
		FCraftSlot Slot;

		for (auto& Field : Link.Value->Fields)
		{
			if (Field.Key.ColumnName == "childId")
			{
				double d;
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

				const FTableRow& Row = GetRow("items", d);
				Slot.Craftable = FItem::ConvertRow(Row, d);
			}
			else if (Field.Key.ColumnName == "shopRevisionId")
			{
				// Todo: Check current revision ID
			}
			else if (Field.Key.ColumnName == "value")
			{
				double d;
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

				Slot.Value = d;
			}
		}

		TArray<TPair<uint32, const FTableRow*>> ConditionLinks = FindLinks("shopCraftConditions", "parentId", ID);

		for (auto& Row : ConditionLinks)
		{
			for (auto& Field : Row.Value->Fields)
			{
				FCraftRequirement Requirement;

				if (Field.Key.ColumnName == "childId")
				{
					double d;
					memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

					const FTableRow& Row = GetRow("items", d);
					Requirement.Item = FItem::ConvertRow(Row, d);
				}
				else if (Field.Key.ColumnName == "shopRevisionId")
				{
					// Todo: Check current revision ID
				}
				else if (Field.Key.ColumnName == "amount")
				{
					double d;
					memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

					Requirement.Amount = d;
				}

				Slot.Requirements.Add(Requirement);
			}
		}

		Slots.Add(Slot);
	}

	return Slots;
}*/
