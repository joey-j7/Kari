#pragma once
#include "GameFramework/Actor.h"
#include "ControlOverwriter.generated.h"

class UDefaultGameInstance;

UCLASS()
class KARI_API AControlOverwriter : public AActor
{
	GENERATED_BODY()

public:
	AControlOverwriter();

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void OvertakeControls();

	UFUNCTION(BlueprintCallable)
	void RestoreControls();

protected:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnMoveX(float Value);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnMoveY(float Value);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnConfirm();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnCancel();

	UPROPERTY(Category = ControlOverwriter, VisibleAnywhere, BlueprintReadOnly)
	UInputComponent* Input = nullptr;

	UDefaultGameInstance* GameInstance = nullptr;
};
