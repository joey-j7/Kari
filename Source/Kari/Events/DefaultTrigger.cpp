#include "DefaultTrigger.h"
#include <Engine/World.h>

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"

ADefaultTrigger::ADefaultTrigger()
{
	OnActorBeginOverlap.AddDynamic(this, &ADefaultTrigger::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ADefaultTrigger::OnOverlapEnd);
}

void ADefaultTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (Cast<ADefaultPlayer>(OtherActor))
		OnEnter();
}

void ADefaultTrigger::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if (Cast<ADefaultPlayer>(OtherActor))
		OnExit();
}

UDefaultGameInstance* ADefaultTrigger::GetGameInstance() const
{
	return Cast<UDefaultGameInstance>(GetWorld()->GetGameInstance());
}
