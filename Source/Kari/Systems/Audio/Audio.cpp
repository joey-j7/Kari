#include "Audio.h"

#include "Sound/SoundWave.h"

#include "AudioDevice.h"
#include "AudioManager.h"

#include "FMODStudioModule.h"
#include "fmod_studio.hpp"
#include "Playlist.h"
#include "AudioUnit.h"
#include "General/DefaultGameInstance.h"

UAudio::UAudio()
{
}

void UAudio::BeginDestroy()
{
	Super::BeginDestroy();

	// Stop channel
	Stop();

	//if (Sound)
	//{
	//	while (!IsLoaded(true)) {}
	//	
	//	Sound->release();
	//	Sound = nullptr;
	//}
}

void UAudio::Init(
	const FString& FilePath,
	UAudioManager* Controller,
	UAudioUnit* Bus,
	bool Stream,
	bool Loop,
	float Vol,
	int32 loopStart,
	int32 loopEnd,
	float PMin,
	float PMax,
	UPlaylist* List,
	AActor* Owner
)
{
	if (!FMODSystem)
	{
		FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
		System->getCoreSystem(&FMODSystem);
	}
	
	if (Sound)
	{
		Sound->release();
		Sound = nullptr;
	}

	Path = FilePath;
	AudioManager = Controller;
	AudioBus = Bus;

	IsStream = Stream;
	Looping = Loop;
	Playlist = List;

	LoopStart = loopStart;
	LoopEnd = loopEnd;

	InitialVolume = Vol;
	LastVolume = Vol;

	PitchMin = FMath::Min(PMin, PMax);
	PitchMax = FMath::Max(PMin, PMax);

	Parent = Owner;

	FMOD_MODE Mode = FMOD_NONBLOCKING;

	if (Owner)
	{
		Mode |= FMOD_3D;
	}

	if (Looping && loopStart >= 0 && loopEnd > loopStart)
	{
		Mode |= FMOD_LOOP_NORMAL;
	}

	if (IsStream)
	{
		Mode |= FMOD_CREATESTREAM;
	}

	SoundLoaded = false;
	FMODSystem->createSound(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "Audio/" + Path)), Mode, nullptr, &Sound);
	
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find audio asset: %s"), *FilePath);
	}

	// Take playlist volume into account
	SetVolume(InitialVolume);

	State = EAudioState::Stopped;
}

void UAudio::Play()
{
	if (State == EAudioState::Playing && DidEverPlay)
		return;

	if (!Sound)
	{
		Init(Path, AudioManager, AudioBus, IsStream, Looping, LastVolume, LoopStart, LoopEnd, PitchMin, PitchMax, Playlist, Parent);

		if (!Sound)
			return;
	}

	if (!IsLoaded())
	{
		DidEverPlay = false;
		return;
	}

	DidEverPlay = true;
	CreateChannel();

	SetVolume(LastVolume);
	
	if (VolumeResult == FMOD_ERR_CHANNEL_STOLEN || VolumeResult == FMOD_ERR_INVALID_HANDLE)
	{
		Stop();
		Play();
		return;
	}

	if (Channel)
	{
		if (PausedSample != UINT_MAX && PausedSample > 0)
		{
			SetSamplePosition(PausedSample);
		}

		Channel->setPaused(false);
		State = EAudioState::Playing;
	}
	else
	{
		State = EAudioState::Stopped;
	}
}

void UAudio::Pause(int32 SamplePosition)
{
	if (State != EAudioState::Playing)
		return;

	State = EAudioState::Paused;

	if (VolumeReachedDelegate.IsBound() || VolumeRampDelay.IsValid())
	{
		OnVolumeReached();
	}

	VolumeReached = true;
	SystemFade = false;

	Channel->removeFadePoints(0, ULLONG_MAX);
	Channel->setPaused(true);

	if (SamplePosition >= 0)
		PausedSample = SamplePosition;
	else
		PausedSample = GetSamplePosition();

	Channel->stop();
	Channel = nullptr;

	SoundLoaded = false;
}

void UAudio::Stop()
{
	if (State == EAudioState::Stopped)
		return;

	State = EAudioState::Stopped;

	NextBeatDelegate.Clear();
	BeatDelay.Invalidate();
	
	NextMeasureDelegate.Clear();
	MeasureDelay.Invalidate();
	
	VolumeReachedDelegate.Clear();
	VolumeRampDelay.Invalidate();

	VolumeReached = true;
	SystemFade = false;

	PausedSample = 0;

	Channel->removeFadePoints(0, ULLONG_MAX);
	Channel->setCallback(nullptr);

	// Channel->setPaused(true);
	Channel->stop();
	Channel = nullptr;

	SoundLoaded = false;

	/*Sound->release();
	Sound = nullptr;*/
}

EAudioState UAudio::GetState() const
{
	return State;
}

void UAudio::SetDelay(float delay)
{
	Delay = delay;
}

void UAudio::Seek(int32 Milliseconds)
{
	if (!Channel)
		return;
	
	while (!IsLoaded(true)) {}
	Channel->setPosition(Milliseconds, FMOD_TIMEUNIT_MS);
}

int32 UAudio::GetPosition()
{
	if (!Channel)
		return 0;

	unsigned Position = 0;

	Channel->getPosition(&Position, FMOD_TIMEUNIT_MS);
	SoundLoaded = false;
	
	return Position;
}

void UAudio::SetSamplePosition(uint32 Samples)
{
	if (!Channel)
		return;
	
	while (!IsLoaded(true)) {}
	Channel->setPosition(Samples, FMOD_TIMEUNIT_PCM);
}

uint32 UAudio::GetSamplePosition()
{
	if (!Channel)
		return 0;

	unsigned Position = 0;
	
	Channel->getPosition(&Position, FMOD_TIMEUNIT_PCM);
	SoundLoaded = false;
	
	return Position;
}

void UAudio::SetVolume(float Vol)
{
	Vol = FMath::Clamp(Vol, 0.01f, 1.f);
	LastVolume = Vol;

	return ApplyVolume();
}

void UAudio::ApplyVolume()
{
	VolumeReachedDelegate.Clear();
	VolumeRampDelay.Invalidate();

	VolumeReached = true;
	SystemFade = false;

	float Vol = LastVolume;

	if (AudioBus)
		Vol *= AudioBus->GetVolume();

	if (Channel)
	{
		Channel->removeFadePoints(0, ULLONG_MAX);
		VolumeResult = Channel->setVolume(Vol);

		if (VolumeResult != FMOD_OK)
		{	
			UE_LOG(LogTemp, Warning, TEXT("FMOD Error while setting volume: %d"), VolumeResult);
		}
	}
}

float UAudio::GetVolume() const
{
	if (!Channel)
		return LastVolume;

	float Volume = GetChannelVolume();

	if (AudioBus)
	{
		Volume = AudioBus->GetVolume() == 0.f ? 0.f : Volume / AudioBus->GetVolume();
	}

	return Volume;
}

float UAudio::GetChannelVolume() const
{
	if (!Channel)
		return LastVolume;

	float Volume;
	Channel->getVolume(&Volume);

	return Volume;
}

void UAudio::FadeVolume(float From, float To, float Duration, bool FadeBySystem)
{
	if (!FadeBySystem && SystemFade)
		return;

	From = FMath::Max(0.0f, From);
	To = FMath::Max(0.01f, To);

	const bool ShouldStop = To == 0.01f;

	if (From == To || From < 0.f || To < 0.f || State != EAudioState::Playing)
		return;

	LastVolume = To;

	if (AudioBus)
	{
		From *= AudioBus->GetVolume();
		To *= AudioBus->GetVolume();
	}

	if (Channel)
	{
		int SampleRate;
		FMODSystem->getSoftwareFormat(&SampleRate, nullptr, nullptr);

		unsigned long long ParentClock;
		Channel->getDSPClock(nullptr, &ParentClock);

		// So we can amplify
		if (GetChannelVolume() == 0.f)
			Channel->setVolume(0.01f);
		
		Channel->removeFadePoints(0, ULLONG_MAX);
		Channel->addFadePoint(ParentClock, 1.f);
		Channel->addFadePoint(ParentClock + Duration * SampleRate, ShouldStop ? 0.01f : To / From);
		
		if (ShouldStop)
		{
			Channel->setDelay(ParentClock, ParentClock + Duration * SampleRate, true);
		}
		
		VolumeReached = false;
		SystemFade = FadeBySystem;

		VolumeReachedDelegate.Clear();

		if (VolumeRampDelay.IsValid())
		{
			VolumeRampDelay.Invalidate();
		}

		AudioManager->GetGameInstance()->GetTimerManager().SetTimer(VolumeRampDelay, this, &UAudio::OnVolumeReached, Duration, false);
	}
}

void UAudio::OnEnd()
{
	Stop();

	if (Playlist)
		Playlist->OnEnd(this);
}

UWorld* UAudio::GetWorld() const
{
	return AudioManager->GetWorld();
}

void UAudio::UpdateNextBeat(int32 BPM)
{
	if (State != EAudioState::Playing)
		return;

	if (!BeatDelay.IsValid())
	{
		const int32 Milliseconds = GetPosition();
		const float Seconds = Milliseconds * 0.001f;

		// Seconds per beat
		const float SPB = 60.f / BPM;
		const float BPS = BPM / 60.f;

		const float Beats = Seconds * BPS;

		const float NextBeat = FMath::FloorToFloat(Beats) + 1.f;
		const float EventTime = SPB * NextBeat;

		const float Timer = EventTime - Seconds;

		AudioManager->GetGameInstance()->GetTimerManager().SetTimer(BeatDelay, this, &UAudio::OnNextBeat, Timer, false);
	}
}

void UAudio::UpdateNextMeasure(int32 BPM)
{
	if (State != EAudioState::Playing)
		return;

	if (!MeasureDelay.IsValid())
	{
		const int32 Milliseconds = GetPosition();
		const float Seconds = Milliseconds * 0.001f;

		// Seconds per beat
		const float SPB = 60.f / BPM;
		const float BPS = BPM / 60.f;

		const float Beats = Seconds * BPS;

		const float NextMeasure = FMath::FloorToFloat(Beats * 0.25f) + 1.f;
		const float EventTime = SPB * NextMeasure * 4.f;

		const float Timer = EventTime - Seconds;

		AudioManager->GetGameInstance()->GetTimerManager().SetTimer(MeasureDelay, this, &UAudio::OnNextMeasure, Timer, false);
	}
}

bool UAudio::IsLoaded(bool Force)
{
	if (!Sound)
		return false;
	
	if (!SoundLoaded || Force)
	{
		FMOD_OPENSTATE state;
		Sound->getOpenState(&state, nullptr, nullptr, nullptr);

		if (
			state != FMOD_OPENSTATE_READY &&
			state != FMOD_OPENSTATE_ERROR &&
			state != FMOD_OPENSTATE_PLAYING
		)
		{
			return false;
		}

		SoundLoaded = true;
	}

	return true;
}

void UAudio::OnNextBeat()
{
	NextBeatDelegate.Broadcast();
	NextBeatDelegate.Clear();

	BeatDelay.Invalidate();
}

void UAudio::OnNextMeasure()
{
	NextMeasureDelegate.Broadcast();
	NextMeasureDelegate.Clear();

	MeasureDelay.Invalidate();
}

void UAudio::OnVolumeReached()
{
	VolumeReachedDelegate.Broadcast();
	VolumeReachedDelegate.Clear();

	VolumeRampDelay.Invalidate();

	VolumeReached = true;
	SystemFade = false;
}

void UAudio::CreateChannel()
{
	if (Channel)
		return;

	FMOD_RESULT result;
	
	if (!Parent)
	{
		result = FMODSystem->playSound(
			Sound,
			nullptr,
			true,
			&Channel
		);
	}
	else
	{
		// TODO: Attach to parent entity
		result = FMODSystem->playSound(
			Sound,
			nullptr,
			true,
			&Channel
		);
	}

	if (Channel)
	{
		SetVolume(LastVolume);

		if (PitchMin != 1.f || PitchMax != 1.f)
		{
			Channel->setPitch(FMath::RandRange(PitchMin, PitchMax));
		}

		if (Looping && (LoopStart > 0 || LoopEnd > LoopStart))
		{
			Channel->setLoopPoints(LoopStart, FMOD_TIMEUNIT_PCM, LoopEnd, FMOD_TIMEUNIT_PCM);
		}

		// Avoids popping
		Channel->setVolumeRamp(true);

		Channel->setUserData(this);
		Channel->setCallback(Callback);
	}
}

void UAudio::PlayBySystem()
{
	if (!PausedBySystem)
		return;

	Play();
	PausedBySystem = false;
}

void UAudio::PauseBySystem()
{
	if (State != EAudioState::Playing)
		return;

	Pause();
	PausedBySystem = true;
}

FMOD_RESULT UAudio::Callback(FMOD_CHANNELCONTROL* chanControl, FMOD_CHANNELCONTROL_TYPE controlType,
	FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void* commandData1, void* commandData2)
{
	if (controlType == FMOD_CHANNELCONTROL_CHANNEL &&
		callbackType == FMOD_CHANNELCONTROL_CALLBACK_END)
	{
		FMOD::Channel* Channel = (FMOD::Channel*)chanControl;

		void* Instance = nullptr;
		Channel->getUserData(&Instance);

		if (!Instance)
			return FMOD_ERR_INVALID_PARAM;

		UAudio* Audio = reinterpret_cast<UAudio*>(Instance);

		if (Audio && Audio->GetState() == EAudioState::Playing)
		{
			Audio->OnEnd();

			const bool IsBGM = Audio->GetPath().Contains(TEXT(".bgm"));

			if (Audio->IsLooping() && !IsBGM)
			{
				Audio->Play();
			}
		}
	}

	return FMOD_OK;
}
