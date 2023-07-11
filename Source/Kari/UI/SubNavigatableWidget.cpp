#include "SubNavigatableWidget.h"
#include "Kismet/GameplayStatics.h"

void USubNavigatableWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();
}

void USubNavigatableWidget::NativeDestruct()
{
	UUserWidget::NativeDestruct();

#if UE_EDITOR
	if (!GetPlayerContext().IsValid())
	{
		return;
	}
#endif

	const auto PlayerController = GetPlayerContext().GetPlayerController();

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController Null"))
		return;
	}


	PlayerController->PopInputComponent(Input);
}

void USubNavigatableWidget::OnSelect_Implementation()
{
	if (!Input)
		Input = NewObject<UInputComponent>();

	Input->BindAxis("NavX", this, &USubNavigatableWidget::OnNavX);
	Input->BindAxis("NavY", this, &USubNavigatableWidget::OnNavY);

	Input->BindAction("Confirm", EInputEvent::IE_Pressed, this, &USubNavigatableWidget::OnConfirm);
	Input->BindAction("Cancel", EInputEvent::IE_Pressed, this, &USubNavigatableWidget::OnCancel);

#if UE_EDITOR
	if (!GetPlayerContext().IsValid())
	{
		return;
	}
#endif

	const auto PlayerController = GetPlayerContext().GetPlayerController();

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController Null"));
		return;
	}


	PlayerController->PushInputComponent(Input);
}

void USubNavigatableWidget::OnCancel_Implementation()
{
	Super::OnCancel_Implementation();

	TimeChanged = 0;
	CurrentMenuOption = 0;

	Input->ClearActionBindings();
	Input->AxisBindings.Reset();

	GetPlayerContext().GetPlayerController()->PopInputComponent(Input);
}