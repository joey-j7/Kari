#include "TimedActor.h"
#include "General/DefaultGameInstance.h"
#include "Events/TimeEvent.h"
#include "Systems/Components/TimeEventComponent.h"
#include "Systems/Time/TimeManager.h"

void ATimedActor::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());

	if (GameInstance)
	{
		TimeManager = GameInstance->TimeManager;

		if (TimeManager)
			TimeManager->Add(this);
	}
}

void ATimedActor::BeginDestroy()
{
	if (TimeManager)
		TimeManager->Remove(this);

	Super::BeginDestroy();
}

void ATimedActor::UpdatePeriod()
{
	OnPeriod(Period, this);
}

void ATimedActor::OnPeriod(EPeriod P, UObject* Caller)
{
	Period = P;
	ITimeable::OnPeriod(Period, Caller);
}
