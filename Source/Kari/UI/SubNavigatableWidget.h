#pragma once
#include "NavigatableWidget.h"
#include "NavigatableWidgetOption.h"

#include "UObject/ScriptInterface.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Array.h"
#include "General/DefaultGameInstance.h"

#include "SubNavigatableWidget.generated.h"


UCLASS()
class KARI_API USubNavigatableWidget : public UNavigatableWidget, public IMenuOption
{
	GENERATED_BODY()

public:
	void OnSelect_Implementation() override;
	void OnCancel_Implementation() override;

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
};