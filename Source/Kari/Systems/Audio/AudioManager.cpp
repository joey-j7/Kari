#include "AudioManager.h"
#include "ActiveSound.h"

#include "General/DefaultGameInstance.h"

UAudioManager::UAudioManager()
{
	MasterBus = NewObject<UAudioUnit>();
	MasterBus->SetAudioManager(this);

	BGMBus = NewObject<UAudioUnit>();
	BGMBus->SetAudioManager(this);

	SFXBus = NewObject<UAudioUnit>();
	SFXBus->SetAudioManager(this);

	AmbienceBus = NewObject<UAudioUnit>();
	AmbienceBus->SetAudioManager(this);

	BGMBus->SetVolume(0.1f);
	BGMBus->SetParent(MasterBus);

	SFXBus->SetVolume(0.4f);
	SFXBus->SetParent(MasterBus);

	AmbienceBus->SetVolume(0.4f);
	AmbienceBus->SetParent(MasterBus);
}

UAudioManager::~UAudioManager()
{
	// Destroy each asset
}

void UAudioManager::BeginPlay(UWorld* W)
{
	World = W;
	SFXs.Empty();

	float SavedMasterUserSetting;
	if (GConfig->GetFloat(TEXT("Audio"), TEXT("MasterVolume"), SavedMasterUserSetting, GGameUserSettingsIni))
	{
		MasterBus->SetUserSetting(SavedMasterUserSetting);
	}
	else
	{
		GConfig->SetFloat(TEXT("Audio"), TEXT("MasterVolume"), 1.0f, GGameUserSettingsIni);
		GConfig->Flush(false);
	}

	float SavedBGMUserSetting;
	if (GConfig->GetFloat(TEXT("Audio"), TEXT("BGMVolume"), SavedBGMUserSetting, GGameUserSettingsIni))
	{
		BGMBus->SetUserSetting(SavedBGMUserSetting);
	}
	else
	{
		GConfig->SetFloat(TEXT("Audio"), TEXT("BGMVolume"), 1.0f, GGameUserSettingsIni);
		GConfig->Flush(false);
	}

	float SavedSFXUserSetting;
	if (GConfig->GetFloat(TEXT("Audio"), TEXT("SFXVolume"), SavedSFXUserSetting, GGameUserSettingsIni))
	{
		SFXBus->SetUserSetting(SavedSFXUserSetting);
	}
	else
	{
		GConfig->SetFloat(TEXT("Audio"), TEXT("SFXVolume"), 1.0f, GGameUserSettingsIni);
		GConfig->Flush(false);
	}

	float SavedAmbienceUserSetting;
	if (GConfig->GetFloat(TEXT("Audio"), TEXT("AmbienceVolume"), SavedAmbienceUserSetting, GGameUserSettingsIni))
	{
		AmbienceBus->SetUserSetting(SavedAmbienceUserSetting);
	}
	else
	{
		GConfig->SetFloat(TEXT("Audio"), TEXT("AmbienceVolume"), 1.0f, GGameUserSettingsIni);
		GConfig->Flush(false);
	}
}

void UAudioManager::Tick(float DeltaTime)
{
	for (int32 i = AmbienceRemoveQueue.Num() - 1; i >= 0 && i < AmbienceRemoveQueue.Num(); --i)
	{
		UPlaylist* Amb = AmbienceRemoveQueue[i];
		bool AllDone = true;

		for (UAudio* Audio : Amb->GetAudioFiles())
		{
			if (Audio->Channel)
			{
				if (!Audio->VolumeReached)
				{
					AllDone = false;
					break;
				}
			}
		}
		
		if (AllDone)
		{
			AmbienceRemoveQueue.RemoveAt(i);
		}
	}

	for (UPlaylist* Playlist : QueuedPlaylists)
	{
		Playlist->Play();
	}

	QueuedPlaylists.RemoveAll([](UPlaylist* Playlist) {
		return Playlist->IsLoaded();
	});
}

UPlaylist* UAudioManager::Play(
	const TArray<FString>& Paths,
	bool Loop,
	int32 LoopStart,
	int32 LoopEnd,
	float PitchMin,
	float PitchMax,
	float Delay,
	float Volume,
	bool Shuffle
)
{
	if (Paths.Num() == 0)
		return nullptr;

	// Add BGM
	if (Paths[0].EndsWith(".bgm"))
	{
		UPlaylist* Music = nullptr;

		// Fade existing BGM
		if (!OverrideBGM)
		{
			if (BGMs.Num() > 0 && BGMs.Top()->GetPaths() != Paths && BGMs.Top()->IsPlaying())
			{
				BGMs.Top()->ClearVolumeEvents();
				BGMs.Top()->FadeVolume(0.f, 1.f, true);
				BGMs.Top()->GetAudioLayer(0)->VolumeReachedDelegate.AddUniqueDynamic(BGMs.Top(), &UPlaylist::Pause);
			}
		}
		else if (OverrideBGM->GetPaths() == Paths)
		{
			OverrideBGM->Play();
			OverrideBGM->FadeVolume(1.f, 1.f, true);

			return OverrideBGM;
		}

		for (UPlaylist* BGM : BGMs)
		{
			if (BGM->GetPaths() == Paths)
			{
				Music = BGM;
				break;
			}
		}
		
		if (Music != nullptr)
		{
			// Add to top
			BGMs.Remove(Music);
			BGMs.Add(Music);

			if (OverrideBGM)
			{
				BGMs.Remove(OverrideBGM);
				BGMs.Add(OverrideBGM);

				return Music;
			}

			Music->Play();

			return Music;
		}

		Music = NewObject<UPlaylist>();
		Music->Init(this, BGMBus, Paths, Loop, LoopStart, LoopEnd, PitchMin, PitchMax, Delay, Volume, Shuffle);

		UAudio* BeatTrack = Music->GetAudioLayer(1);
		if (BeatTrack) BeatTrack->SetVolume(0.01f);

		if (OverrideBGM)
		{
			BGMs.Add(Music);

			BGMs.Remove(OverrideBGM);
			BGMs.Add(OverrideBGM);

			return Music;
		}

		Music->Play();
		BGMs.Add(Music);

		return Music;
	}

	// Add ambience
	if (Paths[0].EndsWith(".amb"))
	{
		for (UPlaylist* Amb : Ambience)
		{
			if (Amb->GetPaths() == Paths)
			{
				UE_LOG(LogTemp, Warning, TEXT("Tried to assign ambience audio while it already exists."));

				Amb->Play();
				return Amb;
			}
		}

		UPlaylist* Amb = NewObject<UPlaylist>();

		Amb->Init(this, AmbienceBus, Paths, Loop, LoopStart, LoopEnd, PitchMin, PitchMax, Delay, Volume, Shuffle);
		Amb->Play();

		Amb->SetVolume(0.f);
		Amb->FadeVolume(1.f, 1.f);

		Ambience.Add(Amb);
		return Amb;
	}

	// Add SFX
	for (UPlaylist* SFX : SFXs)
	{
		if (SFX->GetPaths() == Paths)
		{
			SFX->Stop();
			SFX->Play();

			return SFX;
		}
	}

	UPlaylist* SFX = NewObject<UPlaylist>();
	SFX->Init(this, SFXBus, Paths, Loop, LoopStart, LoopEnd, PitchMin, PitchMax, Delay, Volume, Shuffle);
	SFX->Play();

	SFXs.Add(SFX);
	return SFX;
}

UPlaylist* UAudioManager::Create(
	const TArray<FString>& Paths,
	bool Loop,
	int32 LoopStart,
	int32 LoopEnd,
	float PitchMin,
	float PitchMax,
	float Delay,
	float Volume,
	bool Shuffle
)
{
	if (Paths.Num() == 0)
		return nullptr;

	// Add BGM
	if (Paths[0].EndsWith(".bgm"))
	{
		for (UPlaylist* BGM : BGMs)
		{
			if (BGM->GetPaths() == Paths)
			{
				return BGM;
			}
		}

		UPlaylist* Music = NewObject<UPlaylist>();
		Music->Init(this, BGMBus, Paths, Loop, LoopStart, LoopEnd, PitchMin, PitchMax, Delay, Volume, Shuffle);

		UAudio* BeatTrack = Music->GetAudioLayer(1);
		if (BeatTrack) BeatTrack->SetVolume(0.01f);

		BGMs.EmplaceAt(0, Music);
		return Music;
	}

	// Add ambience
	if (Paths[0].EndsWith(".amb"))
	{
		for (UPlaylist* Amb : Ambience)
		{
			if (Amb->GetPaths() == Paths)
			{
				UE_LOG(LogTemp, Warning, TEXT("Tried to assign ambience audio while it already exists."));
				return Amb;
			}
		}

		UPlaylist* Amb = NewObject<UPlaylist>();

		Amb->Init(this, AmbienceBus, Paths, Loop, LoopStart, LoopEnd, PitchMin, PitchMax, Delay, 0.f, Shuffle);
		
		Ambience.EmplaceAt(0, Amb);
		return Amb;
	}

	// Add SFX
	for (UPlaylist* SFX : SFXs)
	{
		if (SFX->GetPaths() == Paths)
		{
			return SFX;
		}
	}

	UPlaylist* SFX = NewObject<UPlaylist>();
	SFX->Init(this, SFXBus, Paths, Loop, LoopStart, LoopEnd, PitchMin, PitchMax, Delay, Volume, Shuffle);
	
	SFXs.EmplaceAt(0, SFX);
	return SFX;
}

void UAudioManager::CleanupBGMs()
{
	// Clean up stopped BGMs
	for (int32 i = BGMs.Num() - 1; i >= 0; --i)
	{
		UPlaylist* Playlist = BGMs[i];

		bool NotPlaying = true;

		for (UAudio* Audio : Playlist->GetAudioFiles())
		{
			if (Audio->GetState() == EAudioState::Playing)
			{
				NotPlaying = false;
				break;
			}
		}

		if (NotPlaying && BGMs[i] != OverrideBGM)
		{
			BGMs.RemoveAt(i);
		}
	}

	WorldBGM = nullptr;
}

void UAudioManager::ClearAmbience()
{
	if (Ambience.Num() != 0)
		AmbienceRemoveQueue.Append(Ambience);

	for (int32 i = 0; i < AmbienceRemoveQueue.Num(); ++i)
	{
		UPlaylist* Amb = AmbienceRemoveQueue[i];
		Amb->FadeVolume(0.f, 1.f);
	}

	Ambience.Empty();
}

void UAudioManager::SetWorldBGM(UPlaylist* BGM)
{
	WorldBGM = BGM;
}

bool UAudioManager::SetFocussedBGM(const TArray<FString>& Paths, float FadeTime, bool Crossfade)
{
	UPlaylist* Old = nullptr;
	UPlaylist* New = nullptr;

	if (BGMs.Num() > 0)
	{
		if (BGMs.Top()->GetPaths() == Paths)
		{
			if (!BGMs.Top()->IsPlaying())
			{
				PlayBGM();
				return true;
			}

			return false;
		}

		Old = BGMs.Top();
	}

	for (UPlaylist* BGM : BGMs)
	{
		if (BGM->GetPaths() == Paths)
		{
			New = BGM;
			BGMs.Remove(BGM);

			break;
		}
	}

	if (!New)
		New = Create(Paths);
	else
		BGMs.Add(New);

	// Prevent conflicts
	New->GetAudioLayer(0)->VolumeReachedDelegate.Clear();
	New->GetAudioLayer(0)->VolumeRampDelay.Invalidate();

	if (New->IsPlaying())
	{
		New->Pause();
	}

	// Fade out Old, Fade in New
	if (Old && Old->IsPlaying())
	{
		Old->ClearVolumeEvents();
		
		Old->FadeVolume(0.01f, FadeTime, true);
		Old->GetAudioLayer(0)->VolumeReachedDelegate.AddUniqueDynamic(Old, &UPlaylist::Pause);
	}
	
	// Only play if not overriden
	if (!OverrideBGM)
	{
		BGMFadeTime = FadeTime;

		if (!Crossfade)
		{
			if (Old && Old->IsPlaying())
				Old->GetAudioLayer(0)->VolumeReachedDelegate.AddUniqueDynamic(this, &UAudioManager::PlayBGM);
			else
				PlayBGM();
		}
		else
		{
			PlayBGM();
		}
	}

	return true;
}

void UAudioManager::SetOverrideBGM(UPlaylist* BGM)
{
	OverrideBGM = BGM;
}

int32 UAudioManager::MsToSamples(int32 Milliseconds, int32 SampleRate)
{
	return SampleRate * (Milliseconds / 1000.f);
}

int32 UAudioManager::TimeToMs(float BPM, int32 Bar, int32 Measure)
{
	// If the tempo is 60 BPM then each beat is 1 second long
	// Assuming 4/4

	if (BPM <= 0 || Bar < 1 || Measure < 1)
		return 0;

	if (Measure >= 4)
	{
		Bar += (Measure - 1) / 4;
		Measure -= 1;
		Measure %= 4;
		Measure += 1;
	}

	const float MeasureDuration = 60.f / BPM;
	return MeasureDuration * (Measure - 1 + (Bar - 1) * 4) * 1000;
}


int32 UAudioManager::TimeToSamples(float BPM, int32 Bar, int32 Measure, int32 SampleRate)
{
	// If the tempo is 60 BPM then each beat is 1 second long
	// Assuming 4/4

	if (BPM <= 0 || Bar < 1 || Measure < 1 || SampleRate <= 0)
		return 0;

	if (Measure >= 4)
	{
		Bar += (Measure - 1) / 4;
		Measure -= 1;
		Measure %= 4;
		Measure += 1;
	}

	const float MeasureDuration = 60.f / BPM;
	return SampleRate * MeasureDuration * (Measure - 1 + (Bar - 1) * 4);
}

int32 UAudioManager::SamplesToMs(int32 Samples, int32 SampleRate)
{
	if (SampleRate <= 0)
		return 0;

	const float Seconds = Samples / static_cast<float>(SampleRate);
	return Seconds * 1000.f;
}

void UAudioManager::PlayBGM()
{
	if (BGMs.Num() == 0)
		return;

	UPlaylist* BGM = BGMs.Top();

	// Fade back in
	if (!BGM->IsPlaying())
		BGM->Play();

	if (BGMFadeTime > 0.f)
	{
		BGM->ClearVolumeEvents();
		BGM->FadeVolume(1.f, BGMFadeTime, true);
		BGMFadeTime = 0.f;
	}
}

void UAudioManager::StopBGM()
{
	UPlaylist* BGM = GetBGM();

	if (!BGM)
	{
		return;
	}

	if (BGM == OverrideBGM)
		return;

	if (BGM == WorldBGM)
		WorldBGM = nullptr;

	BGM->Stop();
	BGMs.Pop();
}

void UAudioManager::UpdateVolumes()
{
	if (MasterBus->IsDirty() || BGMBus->IsDirty())
	{
		for (UPlaylist* BGM : BGMs)
		{
			for (UAudio* Audio : BGM->GetAudioFiles())
			{
				Audio->ApplyVolume();
			}
		}

		BGMBus->Clean();
	}

	if (MasterBus->IsDirty() || SFXBus->IsDirty())
	{
		for (UPlaylist* SFX : SFXs)
		{
			for (UAudio* Audio : SFX->GetAudioFiles())
			{
				Audio->ApplyVolume();
			}
		}

		SFXBus->Clean();
	}

	if (MasterBus->IsDirty() || AmbienceBus->IsDirty())
	{
		for (UPlaylist* Amb : Ambience)
		{
			for (UAudio* Audio : Amb->GetAudioFiles())
			{
				Audio->ApplyVolume();
			}
		}

		AmbienceBus->Clean();
	}

	MasterBus->Clean();
}

void UAudioManager::TogglePause(bool Paused)
{
	FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);

	if (!System)
		return;

	FMOD::System* FMODSystem = nullptr;
	System->getCoreSystem(&FMODSystem);

	if (!FMODSystem)
		return;

	if (Paused)
		FMODSystem->lockDSP();

	FMOD::ChannelGroup* Group = nullptr;
	FMODSystem->getMasterChannelGroup(&Group);

	if (Group)
		Group->setPaused(Paused);

	if (!Paused)
		FMODSystem->unlockDSP();
}