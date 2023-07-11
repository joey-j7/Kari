#pragma once


#pragma once

#include "UI/SubNavigatableWidget.h"
#include "General/DefaultGameInstance.h"

#include "InventoryTabWidget.generated.h"

class UInventory;

UCLASS()
class KARI_API UInventoryTabWidget : public USubNavigatableWidget
{
	GENERATED_BODY()

public:
	UInventoryTabWidget();

	UFUNCTION(BlueprintNativeEvent)
	void Drop();

protected:
	void NativeOnInitialized() override;
	void OnSelect_Implementation() override;
	void OnCancel_Implementation() override;

	UPROPERTY(Category = InventoryUI, BlueprintReadOnly)
	UInventory* Inventory;

	UPROPERTY(Category = InventoryUI, BlueprintReadOnly, Meta = (ExposeOnSpawn = true))
	int32 TabIndex;
};