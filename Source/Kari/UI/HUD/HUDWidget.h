#pragma once

#include "Blueprint/UserWidget.h"
#include "Systems/Time/Period.h"

#include "HUDWidget.generated.h"

class UDefaultGameInstance;
class UInventory;
class UTimeManager;

UCLASS()
class KARI_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void NativeOnInitialized() override;
	void NativeConstruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	FString Time = "00:00";

	UPROPERTY(BlueprintReadOnly)
	FString Date = "01/01/1970";

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EPeriod> Period = E_AFTERNOON;
	
	UPROPERTY(BlueprintReadOnly)
	float NormPeriodTime = 0.f;
	
	UPROPERTY(BlueprintReadOnly)
	UTimeManager* TimeManager = nullptr;

	UDefaultGameInstance* GameInstance = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UInventory* Inventory = nullptr;
};
