#pragma once
#include "GameFramework/Actor.h"

#include "FadeController.generated.h"

class APlayerCameraManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFadeEventDelegate_OnScreenFaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFadeEventDelegate_OnScreenClear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFadeEventDelegate_OnAudioFaded);

UCLASS()
class KARI_API AFadeController : public AActor
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ManualTick(float DeltaSeconds);

	UFUNCTION(BlueprintCallable)
	void ScreenFadeOut(float Delay);

	UFUNCTION(BlueprintCallable)
	void ScreenFadeIn(float Delay);

	UFUNCTION(BlueprintCallable)
	void SetScreenFade(float Amount);

	UFUNCTION(BlueprintCallable)
	void AudioFadeOut() {}

	UFUNCTION(BlueprintCallable)
	void AudioFadeIn() {}

	UPROPERTY(BlueprintAssignable, Category = "FadeController")
	FFadeEventDelegate_OnScreenFaded OnScreenFaded;

	UPROPERTY(BlueprintAssignable, Category = "FadeController")
	FFadeEventDelegate_OnScreenFaded OnScreenClear;

	UPROPERTY(BlueprintAssignable, Category = "FadeController")
	FFadeEventDelegate_OnAudioFaded OnAudioFaded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeSpeed = 2.f;

protected:

	void ScreenFadeOutImpl(float TimerRate);
	void ScreenFadeOutImpl();

	void ScreenFadeInImpl(float TimerRate);
	void ScreenFadeInImpl();

	bool IsFadeOut = false;

	bool ShouldCallFadeOutEvent = false;
	bool ShouldCallFadeInEvent = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Opacity = 0.0f;

	UPROPERTY()
	APlayerCameraManager* CameraManager = nullptr;

	UPROPERTY()
	FTimerHandle ScreenTimerHandle;
};