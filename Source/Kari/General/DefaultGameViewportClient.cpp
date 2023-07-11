#include "DefaultGameViewportClient.h"

#include "General/DefaultGameInstance.h"

void UDefaultGameViewportClient::LostFocus(FViewport* Viewport)
{
	Super::LostFocus(Viewport);
	Cast<UDefaultGameInstance>(GameInstance)->OnLostFocus.Broadcast();
}

void UDefaultGameViewportClient::ReceivedFocus(FViewport* Viewport)
{
	Super::ReceivedFocus(Viewport);
	Cast<UDefaultGameInstance>(GameInstance)->OnReceivedFocus.Broadcast();
}