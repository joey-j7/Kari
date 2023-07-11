#pragma once
#include "Engine/GameViewportClient.h"
#include "DefaultGameViewportClient.generated.h"

UCLASS()
class KARI_API UDefaultGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	virtual void LostFocus(FViewport* Viewport) override;
	virtual void ReceivedFocus(FViewport* Viewport) override;
};