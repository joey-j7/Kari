#pragma once

#include "CoreMinimal.h"
#include "Queue.h"

#include "Playlist.h"
#include "AudioUnit.h"

#include "AudioManager.generated.h"

class UDefaultGameInstance;

UCLASS()
class KARI_API UAudioManager : public UObject
{
	GENERATED_BODY()

	friend class UAudio;
	friend class UPlaylist;

public:
	UAudioManager();
	~UAudioManager();

	void BeginPlay(UWorld* World);

	void Tick(float DeltaSeconds);

	UFUNCTION(BlueprintCallable)
	UPlaylist* Play(
		const TArray<FString>& Path,
		bool Loop = false,
		int32 LoopStart = 0,
		int32 LoopEnd = 0,
		float PitchMin = 1.f,
		float PitchMax = 1.f,
		float Delay = 0.f,
		float Volume = 1.f,
		bool Shuffle = false
	);

	UFUNCTION(BlueprintCallable)
	UPlaylist* Create(
		const TArray<FString>& Path,
		bool Loop = false,
		int32 LoopStart = 0,
		int32 LoopEnd = 0,
		float PitchMin = 1.f,
		float PitchMax = 1.f,
		float Delay = 0.f,
		float Volume = 1.f,
		bool Shuffle = false
	);

	UFUNCTION(BlueprintCallable)
	void CleanupBGMs();

	UFUNCTION(BlueprintCallable)
	void ClearAmbience();

	UFUNCTION(BlueprintCallable)
	UPlaylist* GetBGM() const { return BGMs.Num() > 0 ? BGMs.Top() : nullptr; }

	UFUNCTION(BlueprintCallable)
	UPlaylist* GetBGMAt(const int32 ID) const { return ID >= 0 && BGMs.Num() > ID ? BGMs[BGMs.Num() - 1 - ID] : nullptr; }

	UFUNCTION(BlueprintCallable)
	UPlaylist* GetWorldBGM() const { return WorldBGM; };

	UFUNCTION(BlueprintCallable)
	void SetWorldBGM(UPlaylist* BGM);
	
	UFUNCTION(BlueprintCallable)
	bool SetFocussedBGM(const TArray<FString>& Paths, float FadeTime = 1.f, bool Crossfade = true);

	UFUNCTION(BlueprintCallable)
	UPlaylist* GetOverrideBGM() const { return OverrideBGM; };

	UFUNCTION(BlueprintCallable)
	void SetOverrideBGM(UPlaylist* BGM);

	UFUNCTION(BlueprintCallable)
	TArray<UPlaylist*> GetAmbience() const { return Ambience; }

	UWorld* GetWorld() const override { return World; };

	UFUNCTION(BlueprintPure)
	static int32 MsToSamples(int32 Milliseconds = 0, int32 SampleRate = 48000);

	UFUNCTION(BlueprintPure)
	static int32 TimeToMs(float BPM = 120.f, int32 Bar = 1, int32 Measure = 1);

	UFUNCTION(BlueprintPure)
	static int32 TimeToSamples(float BPM = 120.f, int32 Bar = 1, int32 Measure = 1, int32 SampleRate = 48000);

	UFUNCTION(BlueprintPure)
	static int32 SamplesToMs(int32 Samples, int32 SampleRate = 48000);

	UFUNCTION(BlueprintCallable)
	void PlayBGM();

	UFUNCTION(BlueprintCallable)
	void StopBGM();

	UDefaultGameInstance* GetGameInstance() const { return Instance; }
	void SetParentInstance(UDefaultGameInstance* I) { Instance = I; }

	void UpdateVolumes();

	UFUNCTION(BlueprintCallable)
	void TogglePause(bool Paused);

protected:
	UWorld* World = nullptr;
	UDefaultGameInstance* Instance = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UAudioUnit* MasterBus = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UAudioUnit* BGMBus = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UAudioUnit* SFXBus = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UAudioUnit* AmbienceBus = nullptr;

	UPROPERTY()
	TArray<UPlaylist*> BGMs;
	float BGMFadeTime = 0.f;

	UPlaylist* WorldBGM = nullptr;
	UPlaylist* OverrideBGM = nullptr;

	UPROPERTY()
	TArray<UPlaylist*> QueuedPlaylists;
	
	UPROPERTY()
	TArray<UPlaylist*> SFXs;

	UPROPERTY()
	TArray<UPlaylist*> Ambience;

	UPROPERTY()
	TArray<UPlaylist*> AmbienceRemoveQueue;
};
