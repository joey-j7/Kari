#include "ItemBase.h"

#include "Characters/Playables/DefaultPlayer.h"
#include "Systems/Inventory/Inventory.h"
#include "UI/HUD/ActionLog/ActionLogWidget.h"
#include "TableDatabase.h"

#include "ConstructorHelpers.h"
#include "General/DefaultGameInstance.h"
#include "General/DefaultPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "Misc/MapErrors.h"
#include "Engine/StaticMesh.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "Kismet/GameplayStatics.h"

AItemBase::AItemBase()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh"));
	RootComponent = MeshComponent;

	TableComponent = CreateDefaultSubobject<UTableComponent>(FName("TableComponent"));
	TableComponent->Init("items", "name");

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(FName("InteractableComponent"));

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("Collider"));
	BoxComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	MeshComponent->SetStaticMesh(Cube.Object);

	UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
	FBoxSphereBounds StaticMeshBounds = StaticMesh->GetBounds();
	BoxComponent->SetBoxExtent(StaticMeshBounds.BoxExtent);

	SetActorTickEnabled(false);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	BoxComponent->SetBoxExtent(FVector(0.0f));
	SetEntity();

	InteractableComponent->OnInteract.AddDynamic(this, &AItemBase::OnInteract_Implementation);	
}

void AItemBase::OnItemUse_Implementation(bool& CanUse_out)
{
	CanUse_out = true;
}

void AItemBase::OnItemEquippedTick_Implementation(float DeltaTime, bool& CanUse_out)
{
	CanUse_out = true;
}

void AItemBase::OnItemPickedUp_Implementation(bool& CanPickUp_out)
{
	CanPickUp_out = true;
}

void AItemBase::OnItemDropped_Implementation(bool& CanDrop_out)
{
	CanDrop_out = Item.Droppable;
}

int32 AItemBase::GetID()
{
	return TableComponent->GetTableID();
}

void AItemBase::SetID(int32 ID)
{
	TableComponent->SetTableID(ID);
	SetEntity();
}

void AItemBase::SetEntity()
{
	InteractableComponent->Action = "Pick up ";

	const FTableRow& row = TableComponent->GetRow("items", TableComponent->GetTableID());
	Item = FItem::ConvertRow(row, TableComponent->GetTableID());

	auto In = GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	InteractableComponent->CharacterName = Item.Name;

	Item.ClassPath = GetClass()->GetFullName();
	Item.ClassPath.RemoveFromStart(TEXT("BlueprintGeneratedClass "));
}

void AItemBase::OnInteract_Implementation()
{
	auto In = GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	// Try picking up the item
	bool Succes = false;
	OnItemPickedUp(Succes);
	if (!Succes)
		return;

	// Try use item
	if (Item.UseOnPickup)
	{
		IsEquipped = true;
		bool itemUsed = false;
		OnItemUse(itemUsed);
		if (itemUsed)
		{
			Destroy();
			return;
		}
	} 

	// Add item to inventory
	bool Success;
	if (Item.Despawnable)
		Success = Instance->Inventory->AddItem(Item);
	else
	{
		Success = Instance->Inventory->AddItemToTab(Instance->Inventory->PersistentTab, Item);
		ensure(Success); // There should never be a moment that you pick up a non-despawnable item when the non-despawnable item tab isn't there, or that the tab is full
	}

	if (Success)
		Destroy();
	else
	{
		IsEquipped = false;
		Instance->GetHUDLog()->AddMessage(Item.ID, "No space in inventory!", EMessageType::E_ITEM_ADD);
	}
}

#if WITH_EDITOR
void AItemBase::CheckForErrors()
{
	Super::CheckForErrors();

	// Get data to check field validity.
	int RowCount = TableDatabase::Get().GetRows("items").Num();
	const FTableField* DeletedField = TableDatabase::Get().GetField("items", "deleted", TableComponent->GetTableID());
	bool Deleted = true;
	if (DeletedField != nullptr)
		memcpy((void*)&Deleted, DeletedField->Data.Get(), DeletedField->Size);

	// Check for a valid item ID
	if (RowCount <= 0)
	{
		FMessageLog("MapCheck").Warning()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString("has an invalid item assigned, but your database appears to be broken! Try syncing with the database to fix this. Inform a programmer if this warning is persistent.")));
	} 
	else if (TableComponent->GetTableID() == UINT_MAX || DeletedField == nullptr)
	{

		FMessageLog("MapCheck").Error()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString("has no item assigned! Did you sync with the database?")));
	} 
	else if (Deleted)
	{
		FMessageLog("MapCheck").Error()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString("has a deleted item assigned to it!")));
	}


	// Check for valid item icon texture
	if (ItemIcon == nullptr)
	{
		FMessageLog("MapCheck").Error()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString("has no icon assigned!")));
	}

}
#endif