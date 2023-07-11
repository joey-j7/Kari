

#pragma once

#include "CoreMinimal.h"
#include "UI/NavigatableWidget.h"
#include "Image.h"

#include "TitleWidget.generated.h"

class UDefaultCraftingComponent;
class UInventory;

USTRUCT(BlueprintType)
struct FSplashScreen
{
	GENERATED_BODY()

	UPROPERTY()
	UImage* Image = nullptr;

	UPROPERTY()
	FColor Background;
};

/**
 * 
 */
UCLASS()
class KARI_API UTitleWidget : public UNavigatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void NativeConstruct() override;

protected:
	void OnInventory() {};
	void OnPause() {};

	TArray<FSplashScreen> SplashScreens;
};
