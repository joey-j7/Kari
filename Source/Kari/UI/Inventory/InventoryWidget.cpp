#include "InventoryWidget.h"

#include "General/DefaultGameInstance.h"
#include "ConstructorHelpers.h"
#include "Systems/Audio/AudioManager.h"

UInventoryWidget::UInventoryWidget(const FObjectInitializer& ObjectInitializer) : UNavigatableWidget(ObjectInitializer)
{
}

void UInventoryWidget::OnNavY_Implementation(float Value)
{
	Super::OnNavY_Implementation(Value);
}

void UInventoryWidget::OnConfirm_Implementation()
{
	Super::OnConfirm_Implementation();
}

void UInventoryWidget::OnCancel_Implementation()
{
	Toggle();
}

void UInventoryWidget::NativeOnInitialized()
{
	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
	Inventory = GameInstance->Inventory;
}

void UInventoryWidget::NativeConstruct()
{
	UNavigatableWidget::NativeConstruct();

	if (Input)
	{
		Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &UInventoryWidget::Toggle);
		Input->BindAction("Pause", EInputEvent::IE_Pressed, this, &UInventoryWidget::OnPause);
		Input->BindAction("QuestLog", EInputEvent::IE_Pressed, this, &UInventoryWidget::OnQuestLog);
		Input->BindAction("Cancel", EInputEvent::IE_Pressed, this, &UInventoryWidget::Toggle);
		Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &UInventoryWidget::Toggle);
	}

	// Slowmotion effect
	GameInstance->SetGameSpeed(0.33f);

	// Play whoosh sound
	GameInstance->AudioManager->Play({ "Global/UI/Inventory/Open" })->SetVolume(0.4f);
}


void UInventoryWidget::Toggle_Implementation()
{
	// Revert Slowmotion effect
	GameInstance->SetGameSpeed(1.f);

	// Play whoosh sound
	GameInstance->AudioManager->Play({ "Global/UI/Inventory/Close" })->SetVolume(0.4f);
}

