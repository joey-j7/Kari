#include "FadeController.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AFadeController::BeginPlay()
{
	Super::BeginPlay();

	CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
}


void AFadeController::ManualTick(float DeltaSeconds)
{
	if (GetWorld()->GetTimerManager().TimerExists(ScreenTimerHandle))
	{
		if (IsFadeOut)
			ScreenFadeOutImpl(DeltaSeconds);
		else
			ScreenFadeInImpl(DeltaSeconds);
	}
}

void AFadeController::ScreenFadeOut(float Delay)
{
	IsFadeOut = true;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DisableInput(nullptr);

	if (PlayerController->GetPawn())
		PlayerController->GetPawn()->DisableInput(nullptr);

	GetWorld()->GetTimerManager().SetTimer(ScreenTimerHandle, this, &AFadeController::ScreenFadeOutImpl, 1.0f / 60.0f, true, Delay);
}

void AFadeController::ScreenFadeIn(float Delay)
{
	IsFadeOut = false;
	GetWorld()->GetTimerManager().SetTimer(ScreenTimerHandle, this, &AFadeController::ScreenFadeInImpl, 1.0f / 60.0f, true, Delay);
}

void AFadeController::SetScreenFade(float Amount)
{
	Opacity = FMath::Clamp(Amount, 0.0f, 1.0f);
	CameraManager->SetManualCameraFade(Opacity, FLinearColor::Black, false);
}

void AFadeController::ScreenFadeOutImpl()
{
	ScreenFadeOutImpl(GetWorld()->GetTimerManager().GetTimerRate(ScreenTimerHandle));
}

void AFadeController::ScreenFadeOutImpl(float TimerRate)
{
	if (ShouldCallFadeOutEvent)
	{
		GetWorld()->GetTimerManager().ClearTimer(ScreenTimerHandle);
		OnScreenFaded.Broadcast();
		OnScreenFaded.Clear();

		ShouldCallFadeOutEvent = false;

		return;
	}

	Opacity += FadeSpeed * TimerRate;
	Opacity = FMath::Min(Opacity, 1.0f);
	CameraManager->SetManualCameraFade(Opacity, FLinearColor::Black, false);

	// Delay OnFaded implementation to next frame, as camera is not applied here
	if (Opacity == 1.0f)
	{
		ShouldCallFadeOutEvent = true;
	}
}

void AFadeController::ScreenFadeInImpl()
{
	ScreenFadeInImpl(GetWorld()->GetTimerManager().GetTimerRate(ScreenTimerHandle));
}

void AFadeController::ScreenFadeInImpl(float TimerRate)
{
	if (ShouldCallFadeInEvent)
	{
		GetWorld()->GetTimerManager().ClearTimer(ScreenTimerHandle);
		OnScreenClear.Broadcast();
		OnScreenClear.Clear();

		ShouldCallFadeInEvent = false;

		return;
	}

	Opacity -= FadeSpeed * TimerRate;
	Opacity = FMath::Max(Opacity, 0.0f);
	CameraManager->SetManualCameraFade(Opacity, FLinearColor::Black, false);

	// Delay OnFaded implementation to next frame, as camera is not applied here
	if (Opacity == 0.0f)
	{
		ShouldCallFadeInEvent = true;
	}
}
