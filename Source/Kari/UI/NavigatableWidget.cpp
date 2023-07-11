#include "NavigatableWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UI/HUD/Log/HUDLogWidget.h"
#include "UI/HUD//Interact/HUDInteractWidget.h"
#include "General/DefaultGameInstance.h"
#include "Systems/Audio/AudioManager.h"

UNavigatableWidget::UNavigatableWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{

}

void UNavigatableWidget::OnNavChange_Implementation(bool Reset)
{
	if (!Reset)
		GameInstance->AudioManager->Play({ "Global/UI/Select" });
}

void UNavigatableWidget::OnEnter_Implementation()
{
}

void UNavigatableWidget::OnExit_Implementation()
{
}

void UNavigatableWidget::ForceFocus()
{

}

void UNavigatableWidget::ResetCurrentOption()
{
	if (CurrentMenuOption != 0)
	{
		CurrentMenuOption = 0;
		OnNavChange(true);
	}
}

void UNavigatableWidget::SetCurrentOption(int32 NewCurrentOption)
{
	if (NewCurrentOption == CurrentMenuOption || NewCurrentOption < 0 || NewCurrentOption >  MenuOptions.Num() - 1)
		return;

	CurrentMenuOption = NewCurrentOption;
	OnNavChange();
}

void UNavigatableWidget::AddHudMessage(int32 ID, FString Message, EMessageType MessageType) const
{
	GameInstance->GetHUDLog()->AddMessage(ID, Message, MessageType);
}

void UNavigatableWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
}

void UNavigatableWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());

	if (!Input)
		Input = NewObject<UInputComponent>();

	Input->BindAxis("NavX", this, &UNavigatableWidget::OnNavX);
	Input->BindAxis("NavY", this, &UNavigatableWidget::OnNavY);

	Input->BindAction("Confirm", EInputEvent::IE_Pressed, this, &UNavigatableWidget::OnConfirm);
	Input->BindAction("Cancel", EInputEvent::IE_Pressed, this, &UNavigatableWidget::OnCancel);

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

	GameInstance->GetHUDInteract()->VisibilityAllowed = false;
}

void UNavigatableWidget::NativeDestruct()
{
	Super::NativeDestruct();

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

	PlayerController->PopInputComponent(Input);

	GameInstance->GetHUDInteract()->VisibilityAllowed = true;
}

void UNavigatableWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UNavigatableWidget::OnNavX_Implementation(float Value)
{
	if (MenuOptions.Num() == 0 || Value == 0.0f)
		return;

	const float Time = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	if (Time - TimeChanged <= ScrollDelay)
		return;

	TimeChanged = Time;

	const int32 Direction = UKismetMathLibrary::SelectInt(-1, 1, NavXInverted);
	const int32 Step = Direction * NavXStepSize;
	const int32 Append = Value > 0.f ? Step : -Step;
	
	PreviousMenuOption = CurrentMenuOption;
	CurrentMenuOption += Append;
	CurrentMenuOption = ((CurrentMenuOption % MenuOptions.Num()) + MenuOptions.Num()) % MenuOptions.Num();

	if (CheckTransparencyOnScroll)
	{
		UWidget* Widget = Cast<UWidget>(
			MenuOptions[CurrentMenuOption].GetObject()
		);

		while (
			Widget && !Widget->IsVisible() &&
			CurrentMenuOption != PreviousMenuOption
			)
		{
			CurrentMenuOption += Append;
			CurrentMenuOption = ((CurrentMenuOption % MenuOptions.Num()) + MenuOptions.Num()) % MenuOptions.Num();
		}
	}

	if (CurrentMenuOption != PreviousMenuOption)
	{
		OnNavChange();
	}
}

float UNavigatableWidget::NavXValue()
{
	return Input->GetAxisValue("NavX");
}

void UNavigatableWidget::OnNavY_Implementation(float Value)
{
	if (MenuOptions.Num() == 0 || Value == 0.0f)
		return;

	const float Time = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	if (Time - TimeChanged <= ScrollDelay)
		return;

	TimeChanged = Time;

	const int32 Direction = UKismetMathLibrary::SelectInt(-1, 1, !NavYInverted);
	const int32 Step = Direction * NavYStepSize;
	const int32 Append = Value > 0.f ? Step : -Step;
	
	PreviousMenuOption = CurrentMenuOption;
	CurrentMenuOption += Append;
	CurrentMenuOption = ((CurrentMenuOption % MenuOptions.Num()) + MenuOptions.Num()) % MenuOptions.Num();

	UWidget* Widget = Cast<UWidget>(
		MenuOptions[CurrentMenuOption].GetObject()
	);

	while (
		Widget && !Widget->IsVisible() &&
		CurrentMenuOption != PreviousMenuOption
	)
	{
		CurrentMenuOption += Append;
		CurrentMenuOption = ((CurrentMenuOption % MenuOptions.Num()) + MenuOptions.Num()) % MenuOptions.Num();
	}
	
	if (CurrentMenuOption != PreviousMenuOption)
	{
		OnNavChange();
	}
}

float UNavigatableWidget::NavYValue()
{
	return Input->GetAxisValue("NavY");
}

void UNavigatableWidget::OnConfirm_Implementation()
{
	if (MenuOptions.Num() <= CurrentMenuOption)
		return;

	if (MenuOptions[CurrentMenuOption].GetObject())
		IMenuOption::Execute_OnSelect(MenuOptions[CurrentMenuOption].GetObject());

	TimeChanged = 0;

	GameInstance->AudioManager->Play({ "Global/UI/Confirm" });
}

void UNavigatableWidget::OnCancel_Implementation()
{
	CurrentMenuOption = 0;
	TimeChanged = 0;

	GameInstance->AudioManager->Play({ "Global/UI/Cancel" });
}


