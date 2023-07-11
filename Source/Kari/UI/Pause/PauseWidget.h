

#pragma once

#include "CoreMinimal.h"
#include "UI/NavigatableWidget.h"
#include "PauseWidget.generated.h"

class UDefaultCraftingComponent;
class UInventory;

/**
 * 
 */
UCLASS()
class KARI_API UPauseWidget : public UNavigatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void NativeConstruct() override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Inventory)
	void Toggle();
	
	void OnCancel_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	int32 CalculateOption(int32 Option, float Value);
	
protected:
	UFUNCTION()
	void OnInventory() {};

	void OnQuestLog() {}

	UPROPERTY(BlueprintReadWrite, Category = Crafting)
	int32 CurrentTabID = 0;

	UPROPERTY(BlueprintReadOnly, Category = Crafting)
	bool IsTabSelected = false;
};
