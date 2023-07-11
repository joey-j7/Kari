#include "BGMTriggerBox.h"
#include <Engine/World.h>

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"

ABGMTriggerBox::ABGMTriggerBox()
{
	OnActorBeginOverlap.AddDynamic(this, &ABGMTriggerBox::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ABGMTriggerBox::OnOverlapEnd);

	PrimaryActorTick.bCanEverTick = false;
}

void ABGMTriggerBox::BeginPlay()
{
	Super::BeginPlay();

	if (!BGMFader)
		BGMFader = NewObject<UBGMFader>(this);

	if (BGMFader)
		BGMFader->Init(
			BGMPath,
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
			Delay
		);
}

void ABGMTriggerBox::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor->IsA(ADefaultPlayer::StaticClass()))
		return;

	if (BGMFader)
	{
		// If it should not revert on overlap end, then it should play on overlap end
		// This is to prevent having the wrong music play when two colliders are too close
		if (RevertOnOverlapEnd)
		{
			BGMFader->Apply();
		}
	}
}

void ABGMTriggerBox::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor->IsA(ADefaultPlayer::StaticClass()))
		return;

	if (BGMFader)
	{
		if (RevertOnOverlapEnd)
		{
			BGMFader->Revert();
		}
		else
		{
			BGMFader->Apply();
		}
	}
}