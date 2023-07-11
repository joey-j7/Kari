#pragma once
#include "Interface.h"
#include "NavigatableWidgetOption.generated.h"

UINTERFACE(BlueprintType)
class KARI_API UMenuOption : public UInterface
{
	GENERATED_BODY()
};

class KARI_API IMenuOption
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = NavigatableUI)
	void OnSelect();
};