


#include "TitleWidget.h"
#include "General/DefaultGameInstance.h"
#include "Kismet/KismetMathLibrary.h"

#include "Systems/Components/Crafting/DefaultCraftingComponent.h"

void UTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Input->BindAction("Pause", EInputEvent::IE_Pressed, this, &UTitleWidget::OnPause);
	Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &UTitleWidget::OnCancel);
	Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &UTitleWidget::OnInventory);
}