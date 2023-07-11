#include "TimeEventComponent.h"
#include "Delegate.h"

FTimeEventDelegate_OnTimeEventBroadcast UTimeEventComponent::OnTimeEventDelegate;

UTimeEventComponent::UTimeEventComponent()
{
	OnTimeEventDelegate.AddUObject(this, &UTimeEventComponent::OnTimeEvent);
}

void UTimeEventComponent::OnTimeEvent(FTimeEvent TimeEvent, bool IsOnEnter)
{
	if (this->EventName == TimeEvent.EventName)
	{
		if (IsOnEnter)
			OnEnter.Broadcast(TimeEvent);
		else 
			OnExit.Broadcast(TimeEvent);
	}
}
