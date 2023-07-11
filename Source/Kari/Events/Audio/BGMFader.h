#pragma once

#include "CoreMinimal.h"

#include "General/DefaultGameInstance.h"

#include "BGMFader.generated.h"

class UPlaylist;

UCLASS(EditInlineNew)
class KARI_API UBGMFader : public UObject
{
	GENERATED_BODY()

public:
	UBGMFader();

	void Init(
		const FString& tPath,
		float tFadeTime = 1.f,
		bool tRevertOnOverlapEnd = true,
		bool tResetOnExit = false,
		bool tLoop = false,
		bool tShuffle = false,
		float tVolume = 1.f,
		int32 tLoopStart = 0,
		int32 tLoopEnd = 0,
		float tPitchMin = 1.f,
		float tPitchMax = 1.f,
		float tDelay = 0.f
	);

	UFUNCTION()
	void Apply();

	UFUNCTION()
	void Revert();

protected:
	UDefaultGameInstance* GameInstance = nullptr;

	UFUNCTION()
	UPlaylist* GetSourceBGM(bool CheckPlaying = true) const;

	UFUNCTION()
	void Fade(bool Override);

	// Differentiates between Apply and Revert
	bool ShouldOverride = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BGMPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPlaylist* TargetBGM = nullptr;

	bool Initialized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeTime = 1.f;

	// Crossfade after the target track has been played once
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RevertOnOverlapEnd = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ResetOnExit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Loop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Shuffle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LoopStart = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LoopEnd = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMin = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMax = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Delay = 0.f;
};