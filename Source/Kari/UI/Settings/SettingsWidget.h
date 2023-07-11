

#pragma once

#include "CoreMinimal.h"
#include "UI/NavigatableWidget.h"
#include "SettingsWidget.generated.h"

class UDefaultCraftingComponent;
class UInventory;

/**
 * 
 */
UCLASS()
class KARI_API USettingsWidget : public UNavigatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void NativeConstruct() override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Inventory)
	void Toggle();
	
	void OnCancel_Implementation() override;

protected:
	UFUNCTION()
	void OnInventory() {};
	
	UFUNCTION()
	void OnPause() {};
};
