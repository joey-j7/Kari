


#include "SettingsWidget.h"
#include "General/DefaultGameInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Systems/Audio/AudioManager.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Disable this button
	Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &USettingsWidget::OnInventory);
	Input->BindAction("QuestLog", EInputEvent::IE_Pressed, this, &USettingsWidget::OnInventory);
	Input->BindAction("Pause", EInputEvent::IE_Pressed, this, &USettingsWidget::OnPause);
	Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &USettingsWidget::OnExit);
}

void USettingsWidget::OnCancel_Implementation()
{
	GameInstance->AudioManager->Play({ "Global/UI/Cancel" });
	Toggle();
}

void USettingsWidget::Toggle_Implementation()
{

}
