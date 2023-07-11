#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerSphere.h"

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"

#include "General/DefaultCameraSystemComponent.h"

#include "CameraShotTrigger.generated.h"

UCLASS(Blueprintable)
class KARI_API ACameraShotTrigger : public ATriggerSphere
{
	GENERATED_BODY()

public:
	ACameraShotTrigger();

	void BeginPlay() override;

	UFUNCTION()
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	UPROPERTY()
	float Offset = 0.f;

protected:
	FVector InitialLocation;
	FRotator InitialRotation;

	ADefaultPlayer* Player = nullptr;

	UPROPERTY(EditAnywhere, Category = "Camera Component")
	UCameraComponent* CameraComponent;
};