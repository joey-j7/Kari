#pragma once

#include "CoreMinimal.h"

#include "Events/Audio/BGMFader.h"
#include "Components/SphereComponent.h"

#include "BGMTriggerSphereComponent.generated.h"

UCLASS(Blueprintable)
class KARI_API UBGMTriggerSphereComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UBGMTriggerSphereComponent();

	void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBGMFader* BGMFader = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	FString BGMPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float FadeTime = 1.f;

	// Crossfade after the target track has been played once
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	bool RevertOnOverlapEnd = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	bool ResetOnExit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	bool Loop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	bool Shuffle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float Volume = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	int32 LoopStart = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	int32 LoopEnd = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float PitchMin = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float PitchMax = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float Delay = 0.f;
};