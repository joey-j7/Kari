#pragma once

#include "CoreMinimal.h"

#include "Audio.h"
#include <TimerManager.h>

#include "Playlist.generated.h"

typedef TArray<UAudio*> PlaylistItem;
typedef TArray<PlaylistItem> Playlists;

class UAudioManager;
class UAudioUnit;
class UDefaultGameInstance;

UCLASS(Blueprintable)
class UPlaylist : public UObject
{
	GENERATED_BODY()

public:
	UPlaylist();

	void Init(
		UAudioManager* AudioManager,
		UAudioUnit* B,
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

	UFUNCTION(BlueprintCallable, Category = Audio)
	void Play();

	UFUNCTION(BlueprintCallable, Category = Audio)
	void Pause();

	UFUNCTION(BlueprintCallable, Category = Audio)
	void Stop();

	UFUNCTION(BlueprintCallable, Category = Audio)
	void Seek(int32 Milliseconds);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void SetVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void ClearVolumeEvents();
	
	UFUNCTION(BlueprintCallable, Category = Audio)
	void FadeVolume(float To, float Duration, bool FadeBySystem = false);
	
	UFUNCTION(BlueprintCallable, Category = Audio)
	bool IsPlaying() const;

	UFUNCTION(BlueprintCallable, Category = Audio)
	bool IsLoaded() const;

	UFUNCTION(BlueprintCallable, Category = Audio)
	bool IsFadingOut() const;

	UFUNCTION(BlueprintCallable, Category = Audio)
	const TArray<FString>& GetPaths() const { return Paths; }

	const PlaylistItem* GetPlaylistItem() const;
	const Playlists& GetPlaylistItems() const { return Lists; }

	UFUNCTION(BlueprintCallable, Category = Audio)
	UAudio* GetAudioLayer(int32 i) const;
	
	UFUNCTION(BlueprintCallable, Category = Audio)
	UAudioUnit* GetBus() const { return AudioBus; };

	UFUNCTION(BlueprintCallable, Category = Audio)
	TArray<UAudio*> GetAudioFiles() const { return AudioFiles; };

	void OnEnd(UAudio* Audio);
	UWorld* GetWorld() const override;

protected:
	void Increment();

	bool Initialized = false;
	bool IsBGM = false;

	bool Loop = false;
	bool Shuffle = false;

	// Gets set on initial volume, used to prevent fading issues
	float MaxVolume = 1.f;

	int32 Index = 0;

	float Delay = 0.f;

	UPROPERTY()
	FTimerHandle DelayHandle;
	FTimerDelegate DelayDelegate;

	UAudioManager* AudioManager = nullptr;
	UAudioUnit* AudioBus = nullptr;

	FMOD::System* FMODSystem = nullptr;

	Playlists Lists;

	UPROPERTY()
	TArray<UAudio*> AudioFiles;

	TArray<FString> Paths;
};
