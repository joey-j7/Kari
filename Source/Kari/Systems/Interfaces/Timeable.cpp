#include "Timeable.h"

#include "Engine/Engine.h"
#include "General/DefaultGameInstance.h"
#include "Kismet/GameplayStatics.h"

void ITimeable::OnPeriod(EPeriod Period, UObject* Caller)
{
	switch (Period)
	{
	case E_NIGHT:
		Execute_OnNight(Caller);
		break;
	case E_MORNING:
		Execute_OnMorning(Caller);
		break;
	case E_AFTERNOON:
		Execute_OnAfternoon(Caller);
		break;
	case E_EVENING:
		Execute_OnEvening(Caller);
		break;
	default:
		break;
	}
}