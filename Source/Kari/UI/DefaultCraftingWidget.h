

#pragma once

#include "CoreMinimal.h"
#include "UI/NavigatableWidget.h"
#include "DefaultCraftingWidget.generated.h"

class UDefaultCraftingComponent;
class UInventory;

/**
 * 
 */
UCLASS()
class KARI_API UDefaultCraftingWidget : public UNavigatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void NativeConstruct() override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Inventory)
	void Toggle();
	
	void OnCancel_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	int32 CalculateOption(int32 Option, float Value) const;
	
	UFUNCTION()
	void SetTradeComponent(UDefaultCraftingComponent* Trade) { TradeComponent = Trade; }

protected:
	UFUNCTION()
	void OnInventory() {};

	UFUNCTION()
	void OnQuestLog() {}

	UFUNCTION()
		void OnNavigateRight();

	UFUNCTION()
		void OnNavigateLeft();

	UPROPERTY(BlueprintReadWrite, Category = Crafting)
	int32 CurrentTabID = 0;

	UPROPERTY(BlueprintReadOnly, Category = Crafting)
	bool IsTabSelected = false;

	UPROPERTY(BlueprintReadOnly, Category = Crafting)	
	UInventory* Inventory = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = Crafting)
	UDefaultCraftingComponent* TradeComponent = nullptr;
};
