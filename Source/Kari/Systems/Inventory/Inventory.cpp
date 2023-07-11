#include "Inventory.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Serialization/BufferArchive.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/HUD/ActionLog/ActionLogWidget.h"
#include "TableObject.h"
#include "TableData.h"
#include "General/DefaultGameInstance.h"
#include "Environment/Props/ItemBase.h"
#include "Characters/Playables/DefaultPlayer.h"
#include "Components/CapsuleComponent.h"
#include "Systems/Quests/QuestLog.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include <string>
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "Environment/Props/ItemFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogInventory);

FTransform GetItemSpawnTransform(ACharacter* Player);

void UInventory::InitCustomTick(UWorld* World)
{
	World->GetTimerManager().SetTimer(TimerHandle, this, &UInventory::CustomTick, TimerPeriod, true);
}

void UInventory::CustomTick()
{
	if (EquippedItem != nullptr)
	{
		// If this item has a timer, decrement it
		if (EquippedItem->GetItem().Effect.Time > 0)
			EquippedItem->Timer = FMath::Max(0.0f, EquippedItem->Timer - TimerPeriod);

		bool TimerExpired = (EquippedItem->Timer == 0);

		// Try to use the item
		bool CanUse;
		EquippedItem->OnItemEquippedTick(TimerPeriod, CanUse);

		// Un-equip item
		if (!CanUse || TimerExpired)
		{
			EquippedItem->Destroy();
			EquippedItem = nullptr;
		}
	}
}

bool UInventory::AddItem(const FItem & item)
{
	FInventorySlot* EmptySlot = nullptr;
	FInventoryTab* EmptySlotTab = nullptr;

	for (auto& Tab : Tabs)
	{
		// Make sure to add non-despawnable items to the non-despawnable items tab, and keep despawnable items out of it
		if ((item.Despawnable && Tab.ID == PersistentTab) || (item.Despawnable == false && Tab.ID != PersistentTab))
			continue;
		
		for (auto& Slot : Tab.Slots)
		{
			if (Slot.Item.ID == item.ID && Slot.Count < Slot.Item.StackLimit)
			{
				Slot.Count++;


				Instance->GetHUDLog()->AddMessage(item.ID, item.Name, EMessageType::E_ITEM_ADD, 1);


				OnInventoryChange();

				return true;
			}

			if (EmptySlot == nullptr && Slot.Count == 0)
			{
				EmptySlotTab = &Tab;
				EmptySlot = &Slot;
			}
		}
	}

	if(EmptySlot)
	{
		auto& Slot = *EmptySlot;
		Slot.Item = item;
		Slot.Count = 1;

#if UE_BUILD_DEVELOPMENT
		if (Slot.Item.StackLimit <= 0)
		{
			Slot.Item.StackLimit = MAX_STACK_COUNT;
			UE_LOG(LogInventory, Error, TEXT("Stack limit is invalid, defaulted to max stack count"));
		}

		check(Instance);
#endif

		Instance->GetHUDLog()->AddMessage(item.ID, item.Name, EMessageType::E_ITEM_ADD, 1);

		OnInventoryChange();

		return true;
	}
	
#if UE_BUILD_DEVELOPMENT
	if (EmptySlot)
	{
		UE_LOG(LogInventory, Error, TEXT("We don't have a empty slot available"));
	}

	check(Instance);
#endif

	return false;
}

bool UInventory::AddItemsByType(int32 ID, int32 Amount, bool DropOnInventoryFull)
{
	UTableObject* TableObject = NewObject<UTableObject>();
	const FTableRow Row = TableObject->GetRow("items", ID);

	if (Row.Fields.Num() == 0)
		return false;

	FItem Item = FItem::ConvertRow(Row, ID);
	return AddItems(Item, Amount, DropOnInventoryFull);
}

bool UInventory::AddItems(const FItem& item, int32 Amount, bool DropOnInventoryFull)
{
#if UE_BUILD_DEVELOPMENT
	if (Amount <= 0)
	{
		UE_LOG(LogInventory, Warning, TEXT("Value provided to amount is illogical"));
		return true;
	}
#endif
	for (auto& Tab : Tabs)
	{		
		// Make sure to add non-despawnable items to the non-despawnable items tab, and keep despawnable items out of it
		if ((item.Despawnable && Tab.ID == PersistentTab) || (item.Despawnable == false && Tab.ID != PersistentTab))
			continue;
		
		const int32 OrigAmount = Amount;

		// Fill up existing slots with similar items
		for (auto& Slot : Tab.Slots)
		{
			if (Slot.Item.ID == item.ID && Slot.Count < Slot.Item.StackLimit)
			{
				int32 Change = FMath::Min(Amount, Slot.Item.StackLimit - Slot.Count);
				Slot.Count += Change;
				Amount -= Change;

				if (Amount <= 0)
				{
					FString Plural = Change > 1 ? "s" : "";
					Instance->GetHUDLog()->AddMessage(item.ID, item.Name, EMessageType::E_ITEM_ADD, OrigAmount);
					OnInventoryChange();

					return true;
				}
			}
		}

		// Fill up new slots when necessary
		if (Amount > 0)
		{
			for (auto& Slot : Tab.Slots)
			{
				if (Slot.Count == 0)
				{
					Slot.Item = item;

					Slot.Count = FMath::Min(Amount, item.StackLimit);
					Amount -= Slot.Count;

					if (Amount <= 0)
					{
						FString Plural = Slot.Count > 1 ? "s" : "";
						Instance->GetHUDLog()->AddMessage(item.ID, item.Name, EMessageType::E_ITEM_ADD, OrigAmount);
						OnInventoryChange();

						return true;
					}
				}

#if UE_BUILD_DEVELOPMENT
				if (Slot.Item.StackLimit <= 0)
				{
					Slot.Item.StackLimit = MAX_STACK_COUNT;
					UE_LOG(LogInventory, Warning, TEXT("Stack limit is invalid, defaulted to max stack count"));
				}
#endif
			}
		}

		// Show pickup message when at least one item has been picked up
		if (Amount != OrigAmount)
		{
			FString Plural = OrigAmount - Amount > 1 ? "s" : "";
			Instance->GetHUDLog()->AddMessage(item.ID, item.Name, EMessageType::E_ITEM_ADD, OrigAmount - Amount);

			OnInventoryChange();
		}
	}

	// Drop items that didn't fit in the inventory on the ground.
	if (DropOnInventoryFull && Amount > 0)
	{
		for (int i = 0; i < Amount; i++) 
		{
			UClass* ItemClass = LoadObject<UClass>(nullptr, *item.ClassPath, nullptr, LOAD_None, nullptr);

			if (!ItemClass)
			{
				ItemClass = LoadItemBlueprint(item.ID);
			}

			FTransform Transform = GetItemSpawnTransform(UGameplayStatics::GetPlayerCharacter(Instance->GetWorld(), 0));
			AItemBase* ActorRef = Instance->GetWorld()->SpawnActorDeferred<AItemBase>(ItemClass, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			check(ActorRef);

			ActorRef->SetID(item.ID);
			ActorRef->Timer = item.Effect.Time;

			ActorRef->IsEquipped = false;

			// Spawn actor
			UGameplayStatics::FinishSpawningActor(ActorRef, Transform);
		}

		FString Plural = Amount > 1 ? "s" : "";
		Instance->GetHUDLog()->AddMessage(item.ID, item.Name, EMessageType::E_ITEM_ADD);
		OnInventoryChange();
	}

	return false;
}

bool UInventory::AddTab(int32 TabID)
{
	// Makes sure that the Tab doesn't exist already
	if (TabExists(TabID))
	{
		return true;
	}
	
	UTableObject* TableObject = NewObject<UTableObject>();
	const FTableData& Dat = TableObject->GetTable("InventoryTabs");

	const FTableRow* TableRow = Dat.Rows.Find(TabID);

#if UE_BUILD_DEVELOPMENT
	if (!TableRow)
	{
		UE_LOG(LogInventory, Error, TEXT("Invalid TabID"));
		return false;
	}
#endif

	FInventoryTab Tab;
	Tab.ID = TabID;

	for (auto& Field : TableRow->Fields)
	{
		if (Field.Key.ColumnName == "Name")
		{
			std::string s;
			s.resize(Field.Value.Size);

			memcpy((void*)s.data(), Field.Value.Data.Get(), Field.Value.Size);

			Tab.Name = UTF8_TO_TCHAR(s.data());
		}
		else if (Field.Key.ColumnName == "SlotCount")
		{
			double d;
			memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

			Tab.SlotCount = d;
		}
		else if (Field.Key.ColumnName == "Unlockable")
		{
			bool b;
			memcpy(&b, Field.Value.Data.Get(), Field.Value.Size);

			Tab.Active = !b;
		}
		else if (Field.Key.ColumnName == "PersistantSlots")
		{
			double d;
			memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);

			Tab.SafeSlots = (int32)d;
		}
	}

	Tab.Slots.SetNum(Tab.SlotCount);
	Tabs.Add(Tab);
	TabFlags |= 1u << TabID;
	return true;
}

bool UInventory::AddItemToTab(int32 TabID, const FItem& item)
{
	// Check if tab is usable
	if (!TabExists(TabID))
	{
		return false;
	}

	FInventoryTab& Tab = GetTabByID(TabID);

	// Increment item slot
	for (auto& Slot : Tab.Slots)
	{
		if (Slot.Item.ID == item.ID && Slot.Count < Slot.Item.StackLimit)
		{
			Slot.Count++;

			if (Instance)
				Instance->GetHUDLog()->AddMessage(item.ID, item.Name, EMessageType::E_ITEM_ADD, 1);

			OnInventoryChange();

			return true;
		}
	}

	for (auto& Slot : Tab.Slots)
	{
		if (Slot.Count == 0)
		{
			Slot.Item = item;
			Slot.Count = 1;

#if UE_BUILD_DEVELOPMENT
			if (Slot.Item.StackLimit <= 0)
			{
				Slot.Item.StackLimit = MAX_STACK_COUNT;
				UE_LOG(LogInventory, Warning, TEXT("Stack limit is invalid, defaulted to max stack count"));
			}

			check(Instance);
#endif

			Instance->GetHUDLog()->AddMessage(item.ID, item.Name, EMessageType::E_ITEM_ADD, 1);

			OnInventoryChange();

			return true;
		}
	}

	return false;
}

bool UInventory::SwapSlots(int32 TabID1, int32 SlotID1, int32 TabID2, int32 SlotID2)
{

	// Check if existing
	FInventoryTab* Tab1 = nullptr;
	FInventoryTab* Tab2 = nullptr;

	for (auto& Tab : Tabs)
	{
		if (Tab.ID == TabID1)
		{
			Tab1 = &Tab;
		}
		else if (Tab.ID == TabID2)
		{
			Tab2 = &Tab;
		}
	}

#if UE_BUILD_DEVELOPMENT

	if (!Tab1 || !Tab2)
	{
		UE_LOG(LogInventory, Error, TEXT("1 of the provided Tab ID's is invalid"));
		return false;
	}
#endif

	FInventorySlot& Slot1 = Tab1->Slots[SlotID1];
	FInventorySlot& Slot2 = Tab2->Slots[SlotID2];
	FInventorySlot Temp = Slot1;
	Slot1 = Slot2;
	Slot2 = Temp;
	

	return true;
}



bool UInventory::RemoveItem(int32 TabID, int32 SlotID)
{
	auto& Tab = GetTabByID(TabID);

	auto& Count = Tab.Slots[SlotID].Count;

	if (Count <= 0)
	{
		return false;
	}

	--Count;
	
	Instance->GetHUDLog()->AddMessage(Tab.Slots[SlotID].Item.ID, Tab.Slots[SlotID].Item.Name, EMessageType::E_ITEM_DROP, 1);

	OnInventoryChange();

	return true;
}

bool UInventory::RemoveItems(int32 TabID, int32 SlotID, int32 Amount)
{
	auto& Tab = GetTabByID(TabID);

	auto& Count = Tab.Slots[SlotID].Count;

	Count = FMath::Max(0, Count - Amount);

	Instance->GetHUDLog()->AddMessage(Tab.Slots[SlotID].Item.ID, Tab.Slots[SlotID].Item.Name, EMessageType::E_ITEM_DROP, Amount);

	OnInventoryChange();

	return true;
}

bool UInventory::RemoveItemsByType(int32 ItemID, int32 Amount)
{
	if (!CheckItemRequirement(ItemID, Amount))
		return false;

	const int32 OrigAmount = Amount;

	for (auto& Tab : Tabs)
	{
		for (auto Slot = Tab.Slots.CreateIterator(); Slot; ++Slot)
		{
			if (ItemID == Slot->Item.ID)
			{
				if (Amount >= Slot->Count)
				{
					Amount -= Slot->Count;
					Slot->Count = 0;
				}
				else
				{
					Slot->Count -= Amount;
					Amount = 0;
					OnInventoryChange();

					return true;
				}
			}
		}
	}

	if (Amount < OrigAmount)
		OnInventoryChange();

	return false;
}

bool UInventory::RemoveTab(int32 TabID)
{
#if UE_BUILD_DEVELOPMENT
	if (!TabExists(TabID))
	{
		UE_LOG(LogInventory, Error, TEXT("Invalid TabID"))
		return false;
	}
#endif
	
	for (int32 i=0; i<Tabs.Num(); i++)
	{
		FInventoryTab& Tab = Tabs[i];
		if (Tab.ID == TabID)
		{
			Tabs.RemoveAt(i);
			break;
		}
	}

	return true;
}

#pragma optimize("", off)
void UInventory::DropItem(const int32 TabID, const int32 SlotID)
{
	//After SpawnActorDeferred Item is set to default object thats why copy of ItemClass and ItemID are needed before hand.

	const FItem& Item = GetSlot(TabID, SlotID).Item;
	const int32 ItemID = Item.ID;
	
	UClass* ItemClass = LoadObject<UClass>(nullptr, *Item.ClassPath, nullptr, LOAD_None, nullptr);

	if (!ItemClass)
	{
		ItemClass = LoadItemBlueprint(Item.ID);
	}

	FTransform Transform = GetItemSpawnTransform(UGameplayStatics::GetPlayerCharacter(Instance->GetWorld(), 0));
	AItemBase* ActorRef = Instance->GetWorld()->SpawnActorDeferred<AItemBase>(ItemClass, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	ActorRef->SetID(ItemID);
	ActorRef->Timer = Item.Effect.Time;
	ActorRef->IsEquipped = false;

	// Spawn actor
	UGameplayStatics::FinishSpawningActor(ActorRef, Transform);

	// Try drop item
	bool itemDropped;
	ActorRef->OnItemDropped(itemDropped);

	if (!itemDropped)
	{
		ActorRef->Destroy();
		return;
	}

	bool succes = RemoveItem(TabID, SlotID);

	check(succes);
}

void UInventory::UseItem(int32 TabID, int32 SlotID)
{
	FItem Item = GetSlotsByID(TabID)[SlotID].Item;
	const int32 ItemID = Item.ID;
	UClass* ItemClass = LoadObject<UClass>(nullptr, *Item.ClassPath, nullptr, LOAD_None, nullptr);

	if (!ItemClass)
	{
		ItemClass = LoadItemBlueprint(Item.ID);
	}

	FTransform Transform = GetItemSpawnTransform(UGameplayStatics::GetPlayerCharacter(Instance->GetWorld(), 0));
	AItemBase* ActorRef = Instance->GetWorld()->SpawnActorDeferred<AItemBase>(ItemClass, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	ActorRef->SetID(ItemID);
	ActorRef->Timer = Item.Effect.Time;
	ActorRef->SetActorHiddenInGame(true);
	ActorRef->SetActorEnableCollision(false);
	ActorRef->IsEquipped = true;

	// Spawn actor
	UGameplayStatics::FinishSpawningActor(ActorRef, Transform);

	// Try use item
	bool itemUsed;
	ActorRef->OnItemUse(itemUsed);

	if (!itemUsed)
	{
		ActorRef->Destroy();
		return;
	}

	OnItemUsed.Broadcast();

	if (Item.Equippable)
	{
		if (EquippedItem != nullptr)
			EquippedItem->Destroy();

		EquippedItem = ActorRef;
		OnItemEquipped.Broadcast();
	}
	else
	{
		ActorRef->Destroy();
	}

	bool succes = RemoveItem(TabID, SlotID);
	check(succes);
}
#pragma optimize("", on)

FInventorySlot UInventory::FindItemByType(int32 ItemID)
{
	for (auto& Tab : Tabs)
	{
		for (auto& Slot : Tab.Slots)
		{
			if (ItemID == Slot.Item.ID)
				return Slot;
		}
	}

	return {};
}

bool UInventory::CheckAvailableItemSlot(int32 ItemID)
{
	FItem ItemProperties = UItemFunctionLibrary::GetItem(ItemID);
		
	for (auto& Tab : Tabs)
	{
		if ((ItemProperties.Despawnable && Tab.ID == PersistentTab) || (ItemProperties.Despawnable == false && Tab.ID != PersistentTab))
			continue;
		
		for (auto& Slot : Tab.Slots)
		{
			if (ItemID == Slot.Item.ID && Slot.Count > 0 && Slot.Count < Slot.Item.StackLimit)
				return true;
		}

		// loop another time to look for empty slots. the reason we have this is because item.ID still exists in the inventory slots.
		for (auto& Slot : Tab.Slots)
		{
			if (Slot.Count == 0)
				return true;
		}
	}
	
	return false;
}

bool UInventory::CheckItemRequirement(int32 ItemID, int32 Amount)
{
	for (auto& Tab : Tabs)
	{
		for (auto& Slot : Tab.Slots)
		{
			if (ItemID == Slot.Item.ID)
			{
				Amount -= Slot.Count;

				if (Amount <= 0)
					return true;
			}
		}
	}

	return false;
}

bool UInventory::CheckCraftRequirement(int32 ItemID, int32 Amount, TArray<FCraftRequirement> Requirements)
{
	FItem ItemProperties = UItemFunctionLibrary::GetItem(ItemID);

	for (auto& Tab : Tabs)
	{
		// Make sure to add non-despawnable items to the non-despawnable items tab, and keep despawnable items out of it
		if ((ItemProperties.Despawnable && Tab.ID == PersistentTab) || (ItemProperties.Despawnable == false && Tab.ID != PersistentTab))
			continue;

		for (auto& Slot : Tab.Slots)
		{
			// Check for empty slots
			if (Slot.Count == 0)
				return true;

			// Check if there is already a slot avialable to place the crafted item in.
			if (ItemID == Slot.Item.ID && Slot.Count > 0 && Slot.Count < Slot.Item.StackLimit)
			{
				Amount -= (Slot.Item.StackLimit - Slot.Count);

				if (Amount <= 0)
					return true;
			}

			// Check for slots that will become empty after carfting.
			for (auto& CraftRequirement : Requirements)
			{
				if (CraftRequirement.Item.ID == Slot.Item.ID)
				{
					float SlotCount = Slot.Count - CraftRequirement.Amount;

					if (SlotCount <= 0)
						return true;

					break;
				}
			}
		}
	}

	return false;
}

bool UInventory::TabExists(int32 TabID)
{
	return ((TabFlags >> TabID) & 1u) != 0;
}


int32 UInventory::CountItem(int32 ItemID)
{
	int32 Count = 0;

	for (auto& Tab : Tabs)
	{
		for (auto& Slot : Tab.Slots)
		{
			if (ItemID == Slot.Item.ID)
			{
				Count += Slot.Count;
			}
		}
	}

	return Count;
}

void UInventory::SellItem(int32 TabID, int32 SlotID)
{
	const FInventorySlot Slot = GetSlot(TabID, SlotID);

	if (Slot.Count <= 0)
		return;

	if (Instance)
		Instance->AddMoney(Slot.Item.SellValue);

	RemoveItem(TabID, SlotID);
}

bool UInventory::SellItems(int32 TabID, int32 SlotID, int32 Amount)
{
	const FInventorySlot Slot = GetSlot(TabID, SlotID);

	if (Slot.Count < Amount)
		return false;

	if (Instance)
		Instance->AddMoney(Slot.Item.SellValue * Amount);

	RemoveItems(TabID, SlotID, Amount);

	return true;
}

void UInventory::OnInventoryChange_Implementation()
{
	if (Instance)
	{
		Instance->QuestLog->UpdateActiveTasks();
		ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(Instance->GetWorld(), 0));
		Player->OnInventoryChange();
	}
}

const TArray<FInventoryTab>& UInventory::GetTabs() const
{
	return Tabs;
}

const FInventorySlot& UInventory::GetSlot(int32 TabID, int32 SlotID)
{
	const FInventoryTab& Tab = GetTabByID(TabID);

	return Tab.Slots[SlotID];
}

const TArray<FInventorySlot>& UInventory::GetSlotsByName(const FString& Name)
{
	return GetTabByName(Name).Slots;
}

const TArray<FInventorySlot>& UInventory::GetSlotsByID(int32 TabID)
{
	return GetTabByID(TabID).Slots;
}

const FInventoryTab& UInventory::GetTabByID(int32 TabID) const
{
	for (auto& Tab : Tabs)
	{
		if (Tab.ID == TabID)
		{
			return Tab;
		}
	}

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogInventory, Error, TEXT("TabID is invalid"));
#endif
	return Tabs[0];
}

FInventoryTab& UInventory::GetTabByID(int32 TabID)
{
	for (auto& Tab : Tabs)
	{
		if (Tab.ID == TabID)
		{
			return Tab;
		}
	}

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogInventory, Error, TEXT("TabID is invalid"));
#endif
	return Tabs[0];

}

const FInventoryTab& UInventory::GetTabByName(const FString& Name) const
{
	const FInventoryTab* Tab = nullptr;
	for (auto& CurrentTab : Tabs)
	{
		if (CurrentTab.Name == Name)
		{
			Tab = &CurrentTab;
			break;
		}
	}

	check(Tab);

	return *Tab;
}

int32 UInventory::GetTabIndexByID(int32 TabID) const
{
	int32 TabIndex = 0;
	for (auto& Tab : Tabs)
	{
		if (Tab.ID == TabID)
		{
			return TabIndex;
		}
		++TabIndex;
	}
#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogInventory, Error, TEXT("Invalid TabID"));
#endif
	return -1;
}

bool UInventory::HasEquippedItem() const
{
	return EquippedItem != nullptr;
}

AItemBase* UInventory::GetEquippedItem() const
{
	return EquippedItem;
}

UClass* UInventory::LoadItemBlueprint(int32 ID)
{
	UClass* Base = AItemBase::StaticClass();

	// Load the asset registry module.
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Ensure asset registry is loaded.
	TArray<FString> ContentPaths;
	ContentPaths.Add(TEXT("/Game/Environment/Props/Pickups"));
	AssetRegistry.ScanPathsSynchronous(ContentPaths);

	// Use the asset registry to get the set of all class names deriving from item base.
	FName BaseClassName = Base->GetFName();

	TArray<FName> BaseNames;
	TSet<FName>Excluded;
	TSet<FName> DerivedNames;

	BaseNames.Add(BaseClassName);
	AssetRegistry.GetDerivedClassNames(BaseNames, Excluded, DerivedNames);

	// Specify the asset type filter.
	FARFilter Filter;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	// Iterate over retrieved blueprint assets
	TArray<TAssetSubclassOf<AItemBase>> Subclasses = {};
	for(auto const& Asset : AssetList)
	{
		// Get the the class this blueprint generates (this is stored as a full path)
		if(auto GeneratedClassPathPtr = Asset.TagsAndValues.Find(TEXT("GeneratedClass")))
		{
			// Convert path to just the name part
			const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
			const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);

			// Check if this class is in the derived set
			if(!DerivedNames.Contains(*ClassName))
				continue;

			// Store using the path to the generated class
			Subclasses.Add(TAssetSubclassOf<AItemBase>(FStringAssetReference(ClassObjectPath)));
		}
	}

	for (const auto& AssetClass : Subclasses)
	{
		if(!AssetClass.IsNull())
		{
			auto LoadedClass = AssetClass.LoadSynchronous();
			AItemBase* Item = Cast<AItemBase>(LoadedClass->GetDefaultObject());

			if (Item != nullptr && Item->GetID() == ID)
			{
				return LoadedClass;
			}
		}
	}

	// If we couldn't load the derived class try loading the base blueprint
	return LoadObject<UClass>(nullptr, TEXT("/Game/Environment/Props/Pickups/BP_ItemBase.BP_ItemBase_C"), nullptr, LOAD_None, nullptr);
}

bool UInventory::SaveData(bool Traversal)
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WriteData(ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result;
	if (Traversal)
	{
		result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Persistence\\InventorySavedData.sav"));
	} else
	{
		result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\InventorySavedData.sav"));
	}

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}

void UInventory::WriteData(FArchive &Ar)
{

	// Save how many tabs are found
	int32 Count = Tabs.Num();
	Ar << Count; //int32

	Ar << TabFlags;
	// Save tabs
	for (auto& Tab : Tabs)
	{
		Tab.SaveLoad(Ar);
	}

	// Save inventory
	for (auto& Tab : Tabs)
	{
		for (auto& Slot : Tab.Slots)
		{
			Slot.SaveLoad(Ar);
		}
	}

	// Save equipped item
	bool StoreEquippedItem = HasEquippedItem();
	Ar << StoreEquippedItem;
	
	if (StoreEquippedItem)
	{
		TArray<uint8> ObjectData;
		FMemoryWriter MemoryWriter(ObjectData, true); 

		FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, false); 
		Archive.ArIsSaveGame = true;
		Archive.ArNoDelta = true;
		EquippedItem->OnItemSaved();
		EquippedItem->Serialize(Archive);

		FItem Item = EquippedItem->GetItem();
		Item.SaveLoad(Ar);

		Ar << ObjectData;
	}
}

bool UInventory::LoadData(bool Traversal)
{
	// Remove previous data
	Tabs.Empty();

	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (Traversal)
	{
		if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Persistence\\InventorySavedData.sav"))) return false;
	}
	else
	{
		if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\InventorySavedData.sav"))) return false;
	}

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadData(FromBinary);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void UInventory::ReadData(FArchive &Ar)
{
	// Load how many tabs are found
	int32 Count;
	Ar << Count; //int32

	// Load tabs
	Ar << TabFlags;

	for (int32 i = 0; i < Count; i++)
	{
		FInventoryTab Tab;
		Tab.SaveLoad(Ar);
		Tabs.Add(Tab);

	}

	for (auto& Tab : Tabs)
	{
		for (int32 i = 0; i < Tab.SlotCount; i++)
		{
			FInventorySlot Slot;
			Slot.SaveLoad(Ar);
			Tab.Slots.Add(Slot);
		}
	}

	// Load equipped item
	bool StoreEquippedItem;
	Ar << StoreEquippedItem;

	// Destroy current equipped item
	if (HasEquippedItem())
		EquippedItem->Destroy();
	EquippedItem = nullptr;

	// Load saved equipped item
	if (StoreEquippedItem)
	{
		// Read item data
		FItem Item;
		Item.SaveLoad(Ar);

		// Spawn actor
		UClass* ItemClass = LoadObject<UClass>(nullptr, *Item.ClassPath, nullptr, LOAD_None, nullptr);
		if (!ItemClass)
		{
			ItemClass = LoadItemBlueprint(Item.ID);
		}
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		EquippedItem = Instance->GetWorld()->SpawnActor<AItemBase>(ItemClass, FTransform::Identity, SpawnParameters);

		// Read actor data
		TArray<uint8> ObjectData;
		Ar << ObjectData;

		FMemoryReader MemoryReader(ObjectData, true);
		FObjectAndNameAsStringProxyArchive Archive(MemoryReader, false); 
		Archive.ArIsSaveGame = true;
		Archive.ArNoDelta = true;

		EquippedItem->Serialize(Archive);
		EquippedItem->Modify();
		EquippedItem->OnItemLoaded();
	}
}

void UInventory::Respawn()
{
	for (auto& Tab : Tabs)
	{
		int32 SafeSlotCount = Tab.SafeSlots;
		for (auto& Slot : Tab.Slots)
		{
			if (SafeSlotCount > 0)
			{
				--SafeSlotCount;
				continue;
			}

			if (Slot.Item.Despawnable == false)
				continue;

			Slot.Count = 0;
		}	
	}
}


FTransform GetItemSpawnTransform(ACharacter* Player)
{
	const FVector PlayerLocation = Player->GetActorLocation();
	const float PlayerRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float PlayerHalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector2D RandomPoint = FMath::RandPointInCircle(PlayerRadius);
	FVector SpawnLocation = PlayerLocation + FVector(RandomPoint.X, RandomPoint.Y, -PlayerHalfHeight);

	// Raycast settings
	FHitResult HitResult;
	FVector Start = SpawnLocation + FVector(0, 0, 10);
	FVector End = SpawnLocation + FVector(0, 0, -100);
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Player);

	FTransform SpawnTransform = FTransform::Identity;
	SpawnTransform.SetTranslation(SpawnLocation);

	// Raycast against the ground
	if (Player->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
	{
		if (HitResult.bBlockingHit)
		{
			FQuat Quat = HitResult.Normal.Rotation().Quaternion();
			Quat *= FQuat(FVector(0, 1, 0), PI * 0.5f);
			SpawnTransform.SetTranslation(HitResult.Location);
			SpawnTransform.SetRotation(Quat);
		}
	}

	return SpawnTransform;
}