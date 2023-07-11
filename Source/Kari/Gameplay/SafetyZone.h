#pragma once
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"

#include "SafetyZone.generated.h"

UCLASS(ClassGroup = (Gameplay))
class KARI_API ASafetyZone : public AActor
{
	GENERATED_BODY()

public:
	ASafetyZone();

	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
	);

	UFUNCTION()
	void OnEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

protected:

	UPROPERTY(Category = SafetyZone, VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* Range = nullptr;
};
