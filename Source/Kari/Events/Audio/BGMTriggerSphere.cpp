#include "BGMTriggerSphere.h"
#include <Engine/World.h>

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"

ABGMTriggerSphere::ABGMTriggerSphere()
{
	OnActorBeginOverlap.AddDynamic(this, &ABGMTriggerSphere::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ABGMTriggerSphere::OnOverlapEnd);

	PrimaryActorTick.bCanEverTick = false;
}

void ABGMTriggerSphere::BeginPlay()
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

void ABGMTriggerSphere::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor->IsA(ADefaultPlayer::StaticClass()))
		return;

	if (BGMFader)
		BGMFader->Apply();
}

void ABGMTriggerSphere::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor->IsA(ADefaultPlayer::StaticClass()))
		return;

	if (BGMFader)
		BGMFader->Revert();
}