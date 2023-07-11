#include "Playlist.h"
#include <FileHelper.h>
#include <Paths.h>

#include <TimerManager.h>
#include <Engine/World.h>
#include "AudioManager.h"
#include "General/DefaultGameInstance.h"

UPlaylist::UPlaylist()
{
}

void UPlaylist::Init(
	UAudioManager* A,
	UAudioUnit* B,
	const TArray<FString>& P,
	bool L,
	int32 LoopStart,
	int32 LoopEnd,
	float PitchMin,
	float PitchMax,
	float D,
	float V,
	bool S
)
{
	if (!FMODSystem)
	{
		FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
		System->getCoreSystem(&FMODSystem);
	}
	
	if (Initialized)
		return;

	AudioManager = A;
	AudioBus = B;

	if (P.Num() > 0)
		IsBGM = P[0].Contains(".bgm");

	Paths = P;
	Loop = L;
	Shuffle = S;
	Delay = D;

	MaxVolume = V >= 0 ? V : 1.f;

	int32 i = 0;

	for (const FString& Path : P)
	{
		FString FullPath = FPaths::ProjectContentDir() + "Audio/" + Path;
		FString FolderPath, PlaylistName;
		FullPath.Split("/", &FolderPath, &PlaylistName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		TArray<FString> Files;
		IFileManager::Get().FindFiles(Files, *FolderPath, TEXT("*.ogg"));

		if (Files.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find playlist assets: %s in %s"), *PlaylistName, *FolderPath);
		}

		FString LocalFolderPath = "";

		if (Path.Contains("/"))
		{
			Path.Split("/", &LocalFolderPath, &PlaylistName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			LocalFolderPath += "/";
		}
		
		for (const FString& File : Files)
		{
			if (!File.StartsWith(PlaylistName + "."))
				continue;

			bool loop = false;
			bool stream = false;

			// If this is the only song in the playlist, make it loop individually
			if (P.Num() == 1)
			{
				loop = Loop;
			}

			// Make streamable for longer sounds
			if (File.Contains(".bgm") || File.Contains(".amb"))
			{
				stream = true;
			}

			if (Lists.Num() <= i)
				Lists.Add({});

			UAudio* Audio = NewObject<UAudio>();
			Audio->Init(LocalFolderPath + File, AudioManager, AudioBus, stream, loop, V, LoopStart, LoopEnd, PitchMin, PitchMax, this);
			
			Lists[i].Add(Audio);
			AudioFiles.Add(Audio);
		}

		i++;
	}

	if (Shuffle)
	{
		Index = FMath::RandRange(0, Lists.Num() - 1);
	}

	Initialized = true;
}

void UPlaylist::Play()
{
	if (Lists.Num() == 0 || DelayHandle.IsValid())
		return;

	if (!IsLoaded())
	{
		AudioManager->QueuedPlaylists.AddUnique(this);
	}
	
	if (Shuffle)
	{
		Index = FMath::RandRange(0, Lists.Num() - 1);
	}

	if (Delay > 0.f)
	{
		DelayDelegate.BindLambda([this]
		{
			if (IsBGM)
				FMODSystem->lockDSP();

			for (UAudio* Layer : Lists[Index])
			{
				Layer->Play();
			}

			if (IsBGM)
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, "Playing \"" + Paths[0] + "\" after delay");

			if (IsBGM)
				FMODSystem->unlockDSP();
		});

		AudioManager->GetGameInstance()->GetTimerManager().SetTimer(DelayHandle, DelayDelegate, Delay, false);
		return;
	}

	for (UAudio* Layer : Lists[Index])
	{
		Layer->CreateChannel();
	}

	if (IsBGM)
		FMODSystem->lockDSP();

	for (UAudio* Layer : Lists[Index])
	{
		const bool WasStopped = Layer->GetState() == EAudioState::Stopped;
		Layer->Play();

		if (WasStopped && Delay < 0.f)
		{
			Layer->Seek(-Delay * 1000.f);
		}
	}

	if (IsBGM)
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, "Playing \"" + Paths[0] + "\"");

	if (IsBGM)
		FMODSystem->unlockDSP();
}

void UPlaylist::Pause()
{
	if (DelayHandle.IsValid())
		AudioManager->GetGameInstance()->GetTimerManager().ClearTimer(DelayHandle);

	if (Lists.Num() == 0)
		return;

	const uint32 SamplePosition = Lists[Index][0]->GetSamplePosition();

	if (IsBGM)
		FMODSystem->lockDSP();

	for (UAudio* Layer : Lists[Index])
	{
		Layer->Pause(SamplePosition);
	}

	if (IsBGM)
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, "Paused \"" + Paths[0] + "\"");

	if (IsBGM)
		FMODSystem->unlockDSP();
}

void UPlaylist::Stop()
{
	if (DelayHandle.IsValid())
		AudioManager->GetGameInstance()->GetTimerManager().ClearTimer(DelayHandle);

	if (Lists.Num() == 0)
		return;

	for (UAudio* Layer : Lists[Index])
	{
		Layer->Stop();
	}

	if (IsBGM)
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, "Stopping \"" + Paths[0] + "\"");
}

void UPlaylist::Seek(int32 Milliseconds)
{
	if (Lists.Num() == 0)
		return;

	if (IsBGM)
		FMODSystem->lockDSP();

	for (UAudio* Layer : Lists[Index])
	{
		Layer->Seek(Milliseconds);
	}

	if (IsBGM)
		FMODSystem->unlockDSP();
}

void UPlaylist::SetVolume(float V)
{
	if (IsBGM)
		FMODSystem->lockDSP();

	for (PlaylistItem Item : Lists)
	{
		for (UAudio* Layer : Item)
		{
			Layer->SetVolume(V * MaxVolume);
		}
	}

	if (IsBGM)
		FMODSystem->unlockDSP();
}

void UPlaylist::ClearVolumeEvents()
{
	for (PlaylistItem Item : Lists)
	{
		for (UAudio* Layer : Item)
		{
			Layer->Channel->removeFadePoints(0, ULLONG_MAX);
			Layer->VolumeReachedDelegate.Clear();

			if (Layer->VolumeRampDelay.IsValid())
			{
				Layer->VolumeRampDelay.Invalidate();
			}
		}
	}
}

void UPlaylist::FadeVolume(float To, float Duration, bool FadeBySystem)
{
	if (IsBGM)
		FMODSystem->lockDSP();

	for (PlaylistItem Item : Lists)
	{
		for (UAudio* Layer : Item)
		{
			Layer->FadeVolume(Layer->GetVolume(), To * MaxVolume, Duration, FadeBySystem);
		}
	}

	if (IsBGM)
		FMODSystem->unlockDSP();
}

bool UPlaylist::IsPlaying() const
{
	for (PlaylistItem Item : Lists)
	{
		for (UAudio* Layer : Item)
		{
			if (Layer->GetState() == EAudioState::Playing)
				return true;
		}
	}

	return false;
}

bool UPlaylist::IsLoaded() const
{
	for (PlaylistItem Item : Lists)
	{
		for (UAudio* Layer : Item)
		{
			if (!Layer->IsLoaded())
				return false;
		}
	}

	return true;
}

bool UPlaylist::IsFadingOut() const
{
	if (Lists.Num() == 0 || Lists[0].Num() == 0)
		return false;

	return (Lists[0][0]->GetVolume() <= 0.01f && !Lists[0][0]->IsVolumeReached());
}

const PlaylistItem* UPlaylist::GetPlaylistItem() const
{
	return static_cast<int32>(Lists.Num()) - 1 < Index ? nullptr : &Lists[Index];
}

UAudio* UPlaylist::GetAudioLayer(int32 i) const
{
	return (!GetPlaylistItem() || static_cast<int32>(Lists[Index].Num()) - 1 < i) ? nullptr : Lists[Index][i];
}

void UPlaylist::OnEnd(UAudio* Audio)
{
	// Check if all layers are stopped
	for (UAudio* Layer : Lists[Index])
	{
		if (Layer->GetState() != EAudioState::Stopped)
		{
			return;
		}
	}

	Increment();
}

UWorld* UPlaylist::GetWorld() const
{
	return AudioManager->GetWorld();
}

void UPlaylist::Increment()
{
	Stop();

	if (Shuffle)
	{
		Index = FMath::RandRange(0, Lists.Num() - 1);
	}
	else
	{
		Index++;

		if (Index >= Lists.Num())
		{
			if (Loop)
			{
				Index %= Lists.Num();
			}
			else
			{
				Index = 0;
				return;
			}
		}
	}

	Stop();
	Play();
}
