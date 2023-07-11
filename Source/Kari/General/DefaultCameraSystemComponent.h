#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "DefaultCameraSystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class KARI_API UDefaultCameraSystemComponent : public USpringArmComponent
{
public:
	GENERATED_BODY()

	UDefaultCameraSystemComponent();

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	UCameraComponent* GetCamera() const { return Camera; }

	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void ResetArmLocation();

	UFUNCTION(BlueprintCallable)
	void Zoom();

	UFUNCTION(BlueprintCallable)
	void Unzoom();

protected:

	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera = nullptr;

	UPROPERTY(Category = PlayerCharacter, VisibleAnywhere, BlueprintReadOnly)
	float MobileArmLength = 1000.f;
	
	UPROPERTY(Category = PlayerCharacter, VisibleAnywhere, BlueprintReadOnly)
	float DefaultArmLength = 2000.f;

	UPROPERTY(Category = PlayerCharacter, EditAnywhere, BlueprintReadWrite)
	float FocussedArmLength = 600.f;

	UPROPERTY(Category = PlayerCharacter, EditAnywhere, BlueprintReadWrite)
	float ZoomSpeed = 40.f;

	bool Zoomed = false;
};
