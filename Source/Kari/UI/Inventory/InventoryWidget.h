

#pragma once

#include "UI/NavigatableWidget.h"
#include "InventoryWidget.generated.h"


class UInventory;

UENUM(BlueprintType)
enum class EItemActionEnum : uint8
{
	USE = 0 UMETA(DisplayName = "Use"),
	MOVE = 1 UMETA(DisplayName = "Move"),
	DROP = 2 UMETA(DisplayName = "Drop")
};

UCLASS()
class KARI_API UInventoryWidget : public UNavigatableWidget
{
	GENERATED_BODY()

public:
	UInventoryWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Inventory)
	void Toggle();

protected:
	void OnNavY_Implementation(float Value) override;
	void OnConfirm_Implementation() override;
	void OnCancel_Implementation() override;

	void NativeOnInitialized() override;
	void NativeConstruct() override;

	UFUNCTION()
	void OnPause() {}

	UFUNCTION()
	void OnQuestLog() {}

	UPROPERTY(Category = InventoryUI, BlueprintReadOnly)
	UInventory* Inventory = nullptr;

	UDefaultGameInstance* GameInstance = nullptr;
};