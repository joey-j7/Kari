#include "ActionLogWidget.h"

#include "Characters/Playables/DefaultPlayer.h"
#include "Systems/Quests/QuestLog.h"
#include "UI/HUD/Interact/HUDInteractWidget.h"

UActionLogWidget::UActionLogWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer) { }

void UActionLogWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UActionLogWidget::AddMessage(int32 ID, const FString& Text, EMessageType MsgType, int32 MsgStack, float Time)
{
	if(MsgType == EMessageType::E_NONE) // Safety net for not showing messages of type None
		return;
	
	if(CurrentMessage.ID >= 0 && CurrentMessage.ID == ID && CurrentMessage.Message == Text && CurrentMessage.MessageType == MsgType)
	{
		CurrentMessage.Stack++;
		CurrentMessage.Timer = Time;
		ShowPopup(CurrentMessage);
		return;
	}

	const FActionLogMessage Message = { ID, Text, MsgType, MsgStack, Time };
	if(CurrentMessage.ID >= 0)
	{	
		if(MsgType == EMessageType::E_ITEM_ADD || MsgType == EMessageType::E_ITEM_DROP)
		{
			ShowPopup(Message);
			return;
		}
	}
	
	Messages.Enqueue(Message);
}

void UActionLogWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (PlayerReference == nullptr) {
		return;
	}
	// 
	if(CurrentID != PlayerReference->GetInteractionCharacterID())
	{
		CurrentID = PlayerReference->GetInteractionCharacterID();
		OnInteractionIDChange(CurrentID);
	}

	// if we are talking don't show the messages yet.
	if(!Messages.IsEmpty() && !Showing && 
	   (PlayerReference->GetInteractionCharacterID() == -INT_MAX || PlayerReference->IsInteractionCharacterTrader()))
	{
		FActionLogMessage Message;
		if(Messages.Dequeue(Message))
		{
			CurrentMessage = Message;
			ShowPopup(Message);
		}
	}		
}

void UActionLogWidget::ShowPopup_Implementation(const FActionLogMessage&) { }

void UActionLogWidget::OnInteractionIDChange_Implementation(int32) { }


void UActionLogWidget::HidePopup()
{
	CurrentMessage.ID = -1;
}

