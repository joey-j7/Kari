#include "DialogueOptionEventComponent.h"

#include "Delegate.h"
#include "Engine/Engine.h"

FDialogueEventDelegate_OnDialogueOptionEventBroadcast UDialogueOptionEventComponent::OnDialogueOptionEventDelegate;

UDialogueOptionEventComponent::UDialogueOptionEventComponent()
{
	OnDialogueOptionEventDelegate.AddUObject(this, &UDialogueOptionEventComponent::OnDialogueOptionEvent);
}

void UDialogueOptionEventComponent::OnDialogueOptionEvent(FDialogueOptionEvent DialogueOptionEvent)
{
	if (this->EventName == DialogueOptionEvent.EventName)
	{
		OnClick.Broadcast(DialogueOptionEvent);
	}
}
