#pragma once

#include "CoreMinimal.h"

#include "Engine/TriggerBox.h"
#include "BGMFader.h"

#include "BGMTriggerBox.generated.h"

UCLASS(Blueprintable)
class KARI_API ABGMTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:
	ABGMTriggerBox();

	void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "BGMFader")
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