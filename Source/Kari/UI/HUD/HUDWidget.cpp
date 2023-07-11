#include "HUDWidget.h"

#include "General/DefaultGameInstance.h"
#include "Systems/Time/TimeManager.h"

#include "Characters/Playables/DefaultPlayer.h"
#include "Systems/Components/Stats/DefaultStatComponent.h"

void UHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
	Inventory = GameInstance->Inventory;
}

void UHUDWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();

	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
	Inventory = GameInstance->Inventory;
	TimeManager = GameInstance->TimeManager;
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!GameInstance)
		return;

	Time = TimeManager->GetTimeString();
	Date = TimeManager->GetDateString();
	Period = TimeManager->GetPeriod();
	NormPeriodTime = TimeManager->GetNormalizedPeriodTime();
}

