#pragma once

#include "CoreMinimal.h"

#include "Object.h"
#include "Environment/Props/Item.h"
#include "Components/ActorComponent.h"
#include "Systems/Interfaces/ICraftable.h"

#include "Inventory.generated.h"

#define INVENTORY_SLOT_COUNT 6
#define INVENTORY_SLOT_COUNT_SAFE 2
#define MAX_STACK_COUNT 99

DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryDelegate_OnItemUsed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryDelegate_OnItemEquipped);

class APlayerCharacter;
typedef TArray<TArray<uint32>> Grid;

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int32 ID = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Slot")
		FItem Item;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Slot")
		int32 Count = 0;

	void SaveLoad(FArchive &Ar)
	{
		Ar << ID;  //int32
		Ar << Count; //int32

		if (Count > 0)
		{
			Item.SaveLoad(Ar);  //Item
		}		
	}
};

USTRUCT(BlueprintType)
struct FInventoryTab
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Tab", meta = (ClampMin = 0))
		int32 ID = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Tab")
		FString Name = "Unnamed Tab";

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Tab")
		int32 SlotCount = INVENTORY_SLOT_COUNT;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Tab")
		bool Active = true;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Tab")
		int32 SafeSlots = INVENTORY_SLOT_COUNT_SAFE;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Tab")
	TArray<FInventorySlot> Slots;

	void SaveLoad(FArchive &Ar)
	{
		Ar << ID;  //int32
		Ar << Name; //string
		Ar << SlotCount; //int32
		Ar << Active; //bool
		Ar << SafeSlots; //bool
	}
};

class UDefaultGameInstance;
class AItemBase;

UCLASS()
class KARI_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventory() = default;
	~UInventory() = default;

	void InitCustomTick(UWorld* World);

	void CustomTick();

	void SetParentInstance(UDefaultGameInstance* I) { Instance = I; }

	/* Add */
	bool AddItem(const FItem& item);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool AddItemsByType(int32 ID, int32 Amount = 1, bool DropOnInventoryFull = true);

	bool AddItems(const FItem& item, int32 Amount, bool DropOnInventoryFull = true);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool AddTab(int32 TabID);

	// Add an item to a tab, followed by actual item reference
	bool AddItemToTab(int32 TabID, const FItem& item);

	/* Swap */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool SwapSlots(int32 TabID1, int32 SlotID1, int32 TabID2, int32 SlotID2);

	/* Delete */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveItem(int32 TabID, int32 SlotID);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveItems(int32 TabID, int32 SlotID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveItemsByType(int32 ItemID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveTab(int32 TabID);

	/* Use/Drop */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void DropItem(int32 TabID, int32 SlotID);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void UseItem(int32 TabID, int32 SlotID);

	/* Find */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	FInventorySlot FindItemByType(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool CheckAvailableItemSlot(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool CheckItemRequirement(int32 ItemID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool CheckCraftRequirement(int32 ItemID, int32 Amount, TArray<FCraftRequirement> Requirements);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool TabExists(int32 TabID);

	/* Count */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	int32 CountItem(int32 ItemID);

	/* Sell */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void SellItem(int32 TabID, int32 SlotID);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool SellItems(int32 TabID, int32 SlotID, int32 Amount);

	/* Events */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Inventory)
	void OnInventoryChange();

	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FInventoryDelegate_OnItemUsed OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FInventoryDelegate_OnItemEquipped OnItemEquipped;

	/* Get */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	const TArray<FInventoryTab>& GetTabs() const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	const FInventorySlot& GetSlot(int32 TabID, int32 SlotID);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	const TArray<FInventorySlot>& GetSlotsByName(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	const TArray<FInventorySlot>& GetSlotsByID(int32 TabID);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	const FInventoryTab& GetTabByID(int32 TabID) const;

	FInventoryTab& GetTabByID(int32 TabID);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	const FInventoryTab& GetTabByName(const FString& Name) const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	int32 GetTabIndexByID(int32 TabID) const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool HasEquippedItem() const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	AItemBase* GetEquippedItem() const;

	UClass* LoadItemBlueprint(int32 ID);

	/* Saving & Loading */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool SaveData(bool Traversal);
	void WriteData(FArchive &Ar);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool LoadData(bool Traversal);
	void ReadData(FArchive &Ar);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void Respawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Inventory)
	int PersistentTab = 6;

private:
	bool Initialized = false;
	UDefaultGameInstance* Instance = nullptr;

	// Tick timer
	FTimerHandle TimerHandle;
	const float TimerPeriod = 1.0f / 60.0f;

	UPROPERTY()
	TArray<FInventoryTab> Tabs;

	uint32 TabFlags = 0;

	UPROPERTY()
	AItemBase* EquippedItem = nullptr;
};
