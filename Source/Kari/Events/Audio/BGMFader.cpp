#include "BGMFader.h"
#include <Engine/World.h>

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/Audio/Playlist.h"
#include "Systems/Audio/AudioManager.h"

UBGMFader::UBGMFader()
{

}

void UBGMFader::Init(
	const FString& Path,
	float tFadeTime,
	bool tRevertOnOverlapEnd,
	bool tResetOnExit,
	bool tLoop,
	bool tShuffle,
	float tVolume,
	int32 tLoopStart,
	int32 tLoopEnd,
	float tPitchMin,
	float tPitchMax,
	float tDelay
)
{
	BGMPath = Path;

	FadeTime = tFadeTime;
	RevertOnOverlapEnd = tRevertOnOverlapEnd;
	ResetOnExit = tResetOnExit;
	Loop = tLoop;
	Shuffle = tShuffle;
	Volume = tVolume;
	LoopStart = tLoopStart;
	LoopEnd = tLoopEnd;
	PitchMin = tPitchMin;
	PitchMax = tPitchMax;
	Delay = tDelay;

	GameInstance = Cast<UDefaultGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if (!GameInstance || Initialized)
		return;

	if (BGMPath.Contains(".bgm"))
	{
		TargetBGM = GameInstance->AudioManager->Create(
			{ BGMPath },
			Loop,
			LoopStart,
			LoopEnd,
			PitchMin,
			PitchMax,
			Delay,
			Volume,
			Shuffle
		);
	}
	else if (!BGMPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("The BGM fader has not been correctly set!"));
	}

	Initialized = true;
}

void UBGMFader::Apply()
{
	if (!Initialized)
		Init(BGMPath,
			FadeTime,
			RevertOnOverlapEnd,
			ResetOnExit,
			Loop,
			Shuffle,
			Volume,
			LoopStart,
			LoopEnd,
			PitchMin,
			PitchMax,
			Delay);

	if (!GameInstance)
		return;

	Fade(true);
}

void UBGMFader::Revert()
{
	if (!Initialized)
		Init(BGMPath,
			FadeTime,
			RevertOnOverlapEnd,
			ResetOnExit,
			Loop,
			Shuffle,
			Volume,
			LoopStart,
			LoopEnd,
			PitchMin,
			PitchMax,
			Delay);

	if (!RevertOnOverlapEnd || !GameInstance)
		return;

	Fade(false);
}

UPlaylist* UBGMFader::GetSourceBGM(bool CheckPlaying) const
{
	UPlaylist* Source = GameInstance->AudioManager->GetOverrideBGM();

	// Cannot fade override BGM
	if (Source)
	{
		return nullptr;
	}

	int32 i = 0;
	Source = GameInstance->AudioManager->GetBGMAt(i);

	if (Source == TargetBGM)
	{
		i++;
		Source = GameInstance->AudioManager->GetBGMAt(i);
	}

	while (Source && CheckPlaying && (!Source->IsPlaying() || Source->IsFadingOut()))
	{
		UPlaylist* S = GameInstance->AudioManager->GetBGMAt(i);

		if (S != TargetBGM)
			Source = S;

		i++;
	}

	return Source == TargetBGM ? nullptr : Source;
}

void UBGMFader::Fade(bool Override)
{
	if (GameInstance->AudioManager->GetOverrideBGM())
		return;

	ShouldOverride = Override;

	UPlaylist* From = Override ? GetSourceBGM() : TargetBGM;
	UPlaylist* To = Override ? TargetBGM : GetSourceBGM(false);

	if (From == To)
		return;

	const bool PlayedBefore = To->GetAudioLayer(0)->GetState() != EAudioState::Stopped;
	const bool Focussed = GameInstance->AudioManager->SetFocussedBGM(To->GetPaths(), FadeTime, PlayedBefore);

	if (Focussed && From && !Override && ResetOnExit)
	{
		To->GetAudioLayer(0)->VolumeReachedDelegate.AddUniqueDynamic(To, &UPlaylist::Stop);
	}
}
