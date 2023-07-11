#include "QuestWidget.h"
#include "Kismet/GameplayStatics.h"

#include "TableField.h"
#include "TableRow.h"
#include "General/DefaultGameInstance.h"

#include <string>
#include "Systems/Components/StoryComponent.h"

#include "Systems/Audio/AudioManager.h"

void UQuestWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Input->BindAction("Pause", EInputEvent::IE_Pressed, this, &UQuestWidget::OnPause);
	Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &UQuestWidget::OnInventory);
	Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &UQuestWidget::OnCancel);
	Input->BindAction("QuestLog", EInputEvent::IE_Pressed, this, &UQuestWidget::OnCancel);

	GameInstance->AudioManager->Play({ "Global/UI/Confirm" });
}

void UQuestWidget::SetQuest(uint32 CurrentID, uint32 acceptID, uint32 denyID)
{
	ID = CurrentID;
	
	const FTableRow& row = StoryComponent->GetRow("quests", CurrentID);
	FString title, description;

	int32 expReward = 0, moneyReward = 0;

	for (auto& field : row.Fields)
	{
		if (field.Value.Size == 0)
			continue;

		if (field.Key == "title")
		{
			std::string s;
			s.resize(field.Value.Size);

			memcpy(const_cast<char*>(s.c_str()), field.Value.Data.Get(), field.Value.Size);
			title = UTF8_TO_TCHAR(s.c_str());
		}
		else if (field.Key == "description")
		{
			std::string s;
			s.resize(field.Value.Size);

			memcpy(const_cast<char*>(s.c_str()), field.Value.Data.Get(), field.Value.Size);
			description = UTF8_TO_TCHAR(s.c_str());
		}
		else if (field.Key == "expReward")
		{
			double d = UINT_MAX;
			memcpy(&d, field.Value.Data.Get(), FMath::Min(sizeof(double), (size_t)field.Value.Size));
			expReward = d;
		}
		else if (field.Key == "moneyReward")
		{
			double d = UINT_MAX;
			memcpy(&d, field.Value.Data.Get(), FMath::Min(sizeof(double), (size_t)field.Value.Size));
			moneyReward = d;
		}
	}

	OnValuesSet(title, description, expReward, moneyReward);
}

void UQuestWidget::OnNavY_Implementation(float Value)
{
	if (Value != 0)
	{
		// TODO why we need instance?
		// UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(GetGameInstance());

		if(static_cast<int32>(Value) != SelectedQuestID)
		{
			SelectedQuestID = static_cast<int32>(Value);
			// MaxPosition.Y = Instance->Inventory->GetSlotsByID(CurrentQuestID).Num();
		}
	}

	Super::OnNavY_Implementation(Value);

	// if (Value != 0 && IsTabSelected)
	// {
		// OnItemChange();
	// }
}


void UQuestWidget::SetStoryComponent(const UStoryComponent& StoryComponent)
{
	// replaced with c++ cast instead of c-style
	this->StoryComponent = const_cast<UStoryComponent*>(&StoryComponent);
}

void UQuestWidget::OnCancel_Implementation()
{
	Super::OnCancel();
	GameInstance->AudioManager->Play({ "Global/UI/Cancel" });
}
