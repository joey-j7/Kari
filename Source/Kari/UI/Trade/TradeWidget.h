

#pragma once

#include "UI/DefaultCraftingWidget.h"
#include "TradeWidget.generated.h"

class UInventory;
class UTradeComponent;

UCLASS()
class KARI_API UTradeWidget : public UDefaultCraftingWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Dialogue)
	void OnItemChange();

protected:
	void OnNavY_Implementation(float Value) override;
	void OnConfirm_Implementation() override;
	void OnCancel_Implementation() override;
};