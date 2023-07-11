#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "HUDInteractWidget.generated.h"

UENUM(BlueprintType)
enum class BUTTON_TYPE : uint8
{
	E_CONFIRM = 0 UMETA(DisplayName = "Confirm"),
	E_CANCEL = 1 UMETA(DisplayName = "Cancel")
};

UCLASS()
class KARI_API UHUDInteractWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD Interact Widget")
	void ShowPopup(BUTTON_TYPE ButtonType, const FString& ActionText = "Interact");

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD Interact Widget")
	void HidePopup();

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD Interact Widget")
	void SetPosition(const FVector2D& Position);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD Interact Widget")
	void ToggleRequirements(AActor* Actor, bool enable = true);
	
	UPROPERTY(BlueprintReadWrite, Category = "HUD Interact Widget")
	bool VisibilityAllowed = true;
};
