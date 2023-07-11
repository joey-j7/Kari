


#include "DefaultCraftingWidget.h"
#include "General/DefaultGameInstance.h"
#include "Kismet/KismetMathLibrary.h"

#include "Systems/Components/Crafting/DefaultCraftingComponent.h"

void UDefaultCraftingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	const auto* instance = Cast<UDefaultGameInstance>(GetGameInstance());
#if UE_BUILD_DEVELOPMENT
	if(!instance)
	{
	    UE_LOG(LogTemp, Fatal, TEXT("Couldn't cast to default game instance!"));
	    return;
	}
#endif
	Inventory = instance->Inventory;

	// Wwise callback, used as a parameter for audio effects
	// FAkAudioDevice::Get()->SetRTPCValue(TEXT("CameraFocus"), -100, 1500, NULL);

	// Slowmotion effect
	GameInstance->SetGameSpeed(0.33f);

	// Play whoosh sound
	// GameInstance->AudioManager->PostEvent(OpenSFX);

	// Disable this button
	Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &UDefaultCraftingWidget::OnInventory);
	Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &UDefaultCraftingWidget::OnCancel);
	Input->BindAction("QuestLog", EInputEvent::IE_Pressed, this, &UDefaultCraftingWidget::OnQuestLog);
	Input->BindAction("CraftingRight", EInputEvent::IE_Pressed, this, &UDefaultCraftingWidget::OnNavigateRight);
	Input->BindAction("CraftingLeft", EInputEvent::IE_Pressed, this, &UDefaultCraftingWidget::OnNavigateLeft);
}

void UDefaultCraftingWidget::OnCancel_Implementation()
{
	Toggle();

#if UE_BUILD_DEVELOPMENT
	if(!TradeComponent)
	{
	    UE_LOG(LogTemp, Fatal, TEXT("There is no trade component!"));
	    return;
	}
#endif 
	IICraftable::Execute_HideMenu(TradeComponent);
}

int32 UDefaultCraftingWidget::CalculateOption(int32 Option, float Value) const
{
	const int32 Direction = UKismetMathLibrary::SelectInt(-1, 1, NavXInverted);
	const int32 Step = Direction * NavXStepSize;
	const int32 NewOption = Option + (Value > 0.f ? Step : -Step);
	return (NewOption + MenuOptions.Num()) % MenuOptions.Num();
}

void UDefaultCraftingWidget::Toggle_Implementation()
{
	// Wwise callback, used as a parameter for audio effects
	// FAkAudioDevice::Get()->SetRTPCValue(TEXT("CameraFocus"), 0, 1500, NULL);

	// Revert Slowmotion effect
	GameInstance->SetGameSpeed(1.f);

	// Play whoosh sound
	// GameInstance->AudioManager->PostEvent(CloseSFX);
}

void UDefaultCraftingWidget::OnNavigateRight() {
	int32 Direction = UKismetMathLibrary::SelectInt(-1, 1, NavXInverted);
	int32 Step = Direction * NavXStepSize;
	OnNavX(-Step);
}

void UDefaultCraftingWidget::OnNavigateLeft() {
	int32 Direction = UKismetMathLibrary::SelectInt(-1, 1, NavXInverted);
	int32 Step = Direction * NavXStepSize;
	OnNavX(Step);
}
