


#include "PauseWidget.h"
#include "General/DefaultGameInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Systems/Audio/AudioManager.h"


void UPauseWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Pause game
	// GameInstance->SetGameSpeed(0.f);

	// Disable this button
	Input->BindAction("Pause", EInputEvent::IE_Pressed, this, &UPauseWidget::OnCancel);
	Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &UPauseWidget::OnInventory);
	Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &UPauseWidget::OnCancel);
	Input->BindAction("QuestLog", EInputEvent::IE_Pressed, this, &UPauseWidget::OnQuestLog);

	GameInstance->AudioManager->Play({ "Global/UI/Confirm" });
}

void UPauseWidget::OnCancel_Implementation()
{
	GameInstance->AudioManager->Play({ "Global/UI/Cancel" });
	Toggle();
}

int32 UPauseWidget::CalculateOption(int32 Option, float Value)
{
	int32 Direction = UKismetMathLibrary::SelectInt(-1, 1, NavXInverted);
	int32 Step = Direction * NavXStepSize;
	Option += Value > 0.f ? Step : -Step;
	return ((Option % MenuOptions.Num()) + MenuOptions.Num()) % MenuOptions.Num();
}

void UPauseWidget::Toggle_Implementation()
{
	// Unpause
	GameInstance->SetGameSpeed(1.f);
}
