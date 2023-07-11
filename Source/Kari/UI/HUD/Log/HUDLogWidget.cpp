#include "HUDLogWidget.h"
#include "General/DefaultGameInstance.h"
#include "Systems/Quests/QuestLog.h"

UHUDLogWidget::UHUDLogWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{

}

void UHUDLogWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UHUDLogWidget::AddMessage(const FString& Text, float Time)
{
	Messages.Add({ Text, Time });

	if (Messages.Num() > 6)
		Messages.RemoveAt(0);
}

void UHUDLogWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	for (int32 i = 0; i < Messages.Num(); ++i)
	{
		auto& Message = Messages[i];

		if (Message.Timer <= 0.f)
		{
			if (Message.FadeTimer <= 0.f)
			{
				Messages.RemoveAt(i);
				i--;
			}

			Message.FadeTimer -= InDeltaTime;
			Message.FadeTimerNorm = Message.FadeTimer / HUD_MESSAGE_FADE_TIME;
		}
		else
		{
			if (Message.FadeTimer < HUD_MESSAGE_FADE_TIME)
			{
				Message.FadeTimer = FMath::Min(Message.FadeTimer + InDeltaTime, HUD_MESSAGE_FADE_TIME);
				Message.FadeTimerNorm = Message.FadeTimer / HUD_MESSAGE_FADE_TIME;
			}
			else
			{
				Message.Timer -= InDeltaTime;
			}
		}
	}
}

void UHUDLogWidget::OnQuestChange_Implementation(const FQuest& Quest) { }
