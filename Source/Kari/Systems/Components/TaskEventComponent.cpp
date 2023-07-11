#include "TaskEventComponent.h"
#include "Delegate.h"

FTaskEventDelegate_OnTaskEventBroadcast UTaskEventComponent::OnTaskEventDelegate;

UTaskEventComponent::UTaskEventComponent()
{
	OnTaskEventDelegate.AddUObject(this, &UTaskEventComponent::OnTaskEvent);
}

void UTaskEventComponent::OnTaskEvent(FTaskEvent TaskEvent, bool IsOnEnter)
{
	if (this->EventName == TaskEvent.EventName)
	{
		if (IsOnEnter)
			OnEnter.Broadcast(TaskEvent);
		else 
			OnExit.Broadcast(TaskEvent);
	}
}
