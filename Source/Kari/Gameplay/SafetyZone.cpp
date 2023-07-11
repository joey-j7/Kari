#include "SafetyZone.h"
#include "Characters/Playables/DefaultPlayer.h"
#include "Systems/Components/Stats/DefaultStatComponent.h"
#include "Systems/Saving/SaveLibrary.h"

ASafetyZone::ASafetyZone()
{
	Range = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));

	Range->SetupAttachment(RootComponent);
	Range->OnComponentBeginOverlap.AddDynamic(this, &ASafetyZone::OnBeginOverlap);
	Range->OnComponentEndOverlap.AddDynamic(this, &ASafetyZone::OnEndOverlap);
}

void ASafetyZone::OnBeginOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ADefaultPlayer* player = Cast<ADefaultPlayer>(OtherActor);
	if (player)
	{
		GetWorld()->GetTimerManager().PauseTimer(player->GetBlessing()->GetTimerHandle());
	}
}

void ASafetyZone::OnEndOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	ADefaultPlayer* player = Cast<ADefaultPlayer>(OtherActor);
	if (player)
	{
		GetWorld()->GetTimerManager().UnPauseTimer(player->GetBlessing()->GetTimerHandle());
	}
}
