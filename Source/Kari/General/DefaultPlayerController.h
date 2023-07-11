#pragma once 

#include "GameFramework/PlayerController.h"

#include "DefaultPlayerController.generated.h"

UCLASS()
class KARI_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADefaultPlayerController();

	virtual bool InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad) override;
	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	bool IsInputEnabled() const { return InputEnabled(); }

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsUsingGamepad = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsMobile = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bResetGamepadDetectionAfterNoInput = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bResetGamepadDetectionAfterNoInput") )
	float GamepadTimeout = 0;

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	float LastGamepadInputTime = 0;
};