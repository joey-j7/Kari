#include "DefaultPlayerController.h"

ADefaultPlayerController::ADefaultPlayerController()
{
#if PLATFORM_IOS || PLATFORM_ANDROID
	bIsMobile = true;
#endif
	bIsUsingGamepad = false;
	bResetGamepadDetectionAfterNoInput = true;
	GamepadTimeout = 5.f;
}

bool ADefaultPlayerController::InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	bool ret = Super::InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
	bIsUsingGamepad = bGamepad;
	if (bGamepad)
	{
		LastGamepadInputTime = GetWorld()->TimeSeconds;
	}
	return ret;
}

bool ADefaultPlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	bool ret = Super::InputKey(Key, EventType, AmountDepressed, bGamepad);
	bIsUsingGamepad = bGamepad;
	if (bGamepad)
	{
		LastGamepadInputTime = GetWorld()->TimeSeconds;
	}
	return ret;
}

void ADefaultPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (IsLocalController() && bResetGamepadDetectionAfterNoInput && bIsUsingGamepad)
	{
		float now = GetWorld()->TimeSeconds;
		if (now > LastGamepadInputTime + GamepadTimeout)
		{
			bIsUsingGamepad = false;
		}
	}
}