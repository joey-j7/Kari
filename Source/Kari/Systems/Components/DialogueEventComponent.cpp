#include "DialogueEventComponent.h"
#include "Delegate.h"
#include "Engine/Engine.h"

FDialogueEventDelegate_OnDialogueEventBroadcast UDialogueEventComponent::OnDialogueEventDelegate;

UDialogueEventComponent::UDialogueEventComponent()
{
	OnDialogueEventDelegate.AddUObject(this, &UDialogueEventComponent::OnDialogueEvent);
}

void UDialogueEventComponent::OnDialogueEvent(FDialogueEvent DialogueEvent, bool IsOnEnter)
{
	if (this->EventName == DialogueEvent.EventName)
	{
		if (IsOnEnter)
		{
			OnEnter.Broadcast(DialogueEvent);
		}
		else
		{
			OnExit.Broadcast(DialogueEvent);
		}
	}
}
