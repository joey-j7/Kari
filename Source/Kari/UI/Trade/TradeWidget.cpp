#include "TradeWidget.h"
#include "Systems/Inventory/Inventory.h"

#include "General/DefaultGameInstance.h"

void UTradeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Input->BindAction("Pause", EInputEvent::IE_Pressed, this, &UTradeWidget::OnInventory);
	Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &UTradeWidget::Toggle);
	Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &UTradeWidget::OnInventory);
}

void UTradeWidget::OnNavY_Implementation(float Value)
{
	UNavigatableWidget::OnNavY_Implementation(Value);
}

void UTradeWidget::OnConfirm_Implementation()
{
	Super::OnConfirm_Implementation();
}

void UTradeWidget::OnCancel_Implementation()
{
	Super::OnCancel_Implementation();
}