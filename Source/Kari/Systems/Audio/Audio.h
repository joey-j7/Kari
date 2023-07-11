#pragma once

#include "CoreMinimal.h"
#include "fmod.hpp"
#include <TimerManager.h>
#include "FMODAudioComponent.h"
#include "Audio.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAudioDelegate);

UENUM(BlueprintType, meta = (Bitmask))
enum class EAudioType : uint8
{
	BGM = 0				UMETA(ToolTip = "Background Music"),
	SFX = 1				UMETA(ToolTip = "Sound Effect"),
	Ambience = 2		UMETA(ToolTip = "Ambience")
};

UENUM(BlueprintType, meta = (Bitmask))
enum class EAudioState : uint8
{
	Stopped = 0			UMETA(ToolTip = "Stopped"),
	Paused = 1			UMETA(ToolTip = "Paused"),
	Playing = 2			UMETA(ToolTip = "Playing")
};

class UAudioManager;
class UPlaylist;
class UAudioUnit;

UCLASS()
class UAudio : public UObject
{
	GENERATED_BODY()

	friend class UAudioManager;
	friend class UPlaylist;

public:
	UAudio();

	void BeginDestroy() override;

	void Init(
		const FString& FilePath,
		UAudioManager* Controller,
		UAudioUnit* AudioBus,
		bool IsStream = false,
		bool Loop = false,
		float Volume = 1.f,
		int32 loopStart = 0,
		int32 loopEnd = 0,
		float PitchMin = 1.f,
		float PitchMax = 1.f,
		UPlaylist* Playlist = nullptr,
		AActor* Owner = nullptr
	);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void Play();

	UFUNCTION(BlueprintCallable, Category = Audio)
	void Pause(int32 SamplePosition = -1);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void Stop();

	UFUNCTION(BlueprintCallable, Category = Audio)
	EAudioState GetState() const;

	UFUNCTION(BlueprintCallable, Category = Audio)
	void SetDelay(float Delay);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void Seek(int32 Milliseconds);

	UFUNCTION(BlueprintCallable, Category = Audio)
	int32 GetPosition();

	void SetSamplePosition(uint32 Samples);
	uint32 GetSamplePosition();

	UFUNCTION(BlueprintCallable, Category = Audio)
	void SetVolume(float Vol);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void ApplyVolume();

	UFUNCTION(BlueprintCallable, Category = Audio)
	float GetVolume() const;

	// Get last set volume
	UFUNCTION(BlueprintCallable, Category = Audio)
	float GetLastVolume() const { return LastVolume; };
	
	UFUNCTION(BlueprintCallable, Category = Audio)
	float GetChannelVolume() const;

	UFUNCTION(BlueprintCallable, Category = Audio)
	void FadeVolume(float From, float To, float Duration, bool FadeBySystem = false);

	UFUNCTION(BlueprintCallable, Category = Audio)
	const FString& GetPath() const
	{
		return Path;
	}

	UFUNCTION(BlueprintCallable, Category = Audio)
	bool IsLooping() const { return Looping;  }

	UFUNCTION(BlueprintCallable, Category = Audio)
	int32 GetLoopStart() const { return LoopStart; }

	UFUNCTION(BlueprintCallable, Category = Audio)
	int32 GetLoopEnd() const { return LoopEnd; }

	void OnEnd();

	UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = Audio)
	void UpdateNextBeat(int32 BPM);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void UpdateNextMeasure(int32 BPM);

	UFUNCTION(BlueprintCallable, Category = Audio)
	UAudioUnit* GetBus() const { return AudioBus; }

	UFUNCTION(BlueprintCallable, Category = Audio)
	bool IsVolumeReached() const { return VolumeReached; }

	UFUNCTION(BlueprintCallable, Category = Audio)
	bool IsLoaded(bool Force = false);
	
	// Called in case a volume ramp is active and the volume has been reached
	UPROPERTY(BlueprintAssignable)
	FAudioDelegate VolumeReachedDelegate;

protected:
	void OnNextBeat();
	void OnNextMeasure();
	void OnVolumeReached();

	void CreateChannel();

	// System pause
	void PlayBySystem();
	void PauseBySystem();
	
	bool PausedBySystem = false;

	UPROPERTY(BlueprintAssignable)
	FAudioDelegate NextBeatDelegate;

	UPROPERTY(BlueprintAssignable)
	FAudioDelegate NextMeasureDelegate;

	UPROPERTY()
	FTimerHandle VolumeRampDelay;

	UPROPERTY()
	FTimerHandle BeatDelay;
	
	UPROPERTY()
	FTimerHandle MeasureDelay;

	FMOD_RESULT VolumeResult;

	static FMOD_RESULT F_CALLBACK Callback(
		FMOD_CHANNELCONTROL* chanControl,
		FMOD_CHANNELCONTROL_TYPE controlType,
		FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
		void* commandData1,
		void* commandData2
	);

	EAudioState State = EAudioState::Stopped;
	FString Path;

	UAudioManager* AudioManager = nullptr;

	UPROPERTY()
	UPlaylist* Playlist = nullptr;
	UAudioUnit* AudioBus = nullptr;

	FMOD::Channel* Channel = nullptr;
	FMOD::Sound* Sound = nullptr;

	FMOD::System* FMODSystem = nullptr;

	FTimerHandle VolumeTimer;

	bool SoundLoaded = false;
	bool DidEverPlay = false;
	
	bool VolumeReached = true;
	bool SystemFade = false;

	uint32 PausedSample = UINT_MAX;

	float InitialVolume = 1.f;
	float LastVolume = 1.f;

	float Delay = 0.f;

	float PitchMin = 1.f;
	float PitchMax = 1.f;

	int32 LoopStart = 0;
	int32 LoopEnd = 0;

	bool Looping = false;
	bool IsStream = false;

	AActor* Parent = nullptr;
};