#include "BGMTriggerSphereComponent.h"
#include <Engine/World.h>

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"

UBGMTriggerSphereComponent::UBGMTriggerSphereComponent()
{
	OnComponentBeginOverlap.AddDynamic(this, &UBGMTriggerSphereComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &UBGMTriggerSphereComponent::OnOverlapEnd);
}

void UBGMTriggerSphereComponent::BeginPlay()
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

void UBGMTriggerSphereComponent::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!OtherActor->IsA(ADefaultPlayer::StaticClass()))
		return;

	if (BGMFader)
		BGMFader->Apply();
}

void UBGMTriggerSphereComponent::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (!OtherActor->IsA(ADefaultPlayer::StaticClass()))
		return;

	if (BGMFader)
		BGMFader->Revert();
}