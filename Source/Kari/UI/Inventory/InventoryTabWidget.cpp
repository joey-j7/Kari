#include "InventoryTabWidget.h"

#include "General/DefaultGameInstance.h"

UInventoryTabWidget::UInventoryTabWidget()
{
}

void UInventoryTabWidget::Drop_Implementation()
{

}

void UInventoryTabWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Inventory = GameInstance->Inventory;
}

void UInventoryTabWidget::OnSelect_Implementation()
{
	Super::OnSelect_Implementation();

	Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &UInventoryTabWidget::OnCancel);
	Input->BindAction("Drop", EInputEvent::IE_Pressed, this, &UInventoryTabWidget::Drop);
}

void UInventoryTabWidget::OnCancel_Implementation()
{
	Super::OnCancel_Implementation();
}
