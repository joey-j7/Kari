#pragma once
#include "GameFramework/Actor.h"

#include "Gate.generated.h"

UCLASS(ClassGroup = (Gameplay))
class KARI_API AGate : public AActor
{
	GENERATED_BODY()

public:
	AGate();

	UFUNCTION(BlueprintImplementableEvent, Category = "Gates")
	bool IsLocked();

	UFUNCTION(BlueprintNativeEvent, Category = "Gates")
	void OnLoadUnlock();
};