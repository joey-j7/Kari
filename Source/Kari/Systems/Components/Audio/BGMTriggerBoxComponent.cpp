#include "BGMTriggerBoxComponent.h"
#include <Engine/World.h>

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"

UBGMTriggerBoxComponent::UBGMTriggerBoxComponent()
{
	OnComponentBeginOverlap.AddDynamic(this, &UBGMTriggerBoxComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &UBGMTriggerBoxComponent::OnOverlapEnd);
}

void UBGMTriggerBoxComponent::BeginPlay()
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

void UBGMTriggerBoxComponent::OnOverlapBegin(
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

void UBGMTriggerBoxComponent::OnOverlapEnd(
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