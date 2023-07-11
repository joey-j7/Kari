#pragma once
#include "NavigatableWidgetOption.h"
#include "HUD/ActionLog/ActionLogWidget.h"

#include "UObject/ScriptInterface.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Array.h"

#include "NavigatableWidget.generated.h"

class UDefaultGameInstance;

UCLASS()
class KARI_API UNavigatableWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UNavigatableWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintImplementableEvent, Category = NavigatableUI)
	void Refresh();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = NavigatableUI)
	void OnNavX(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = NavigatableUI)
	void OnNavY(float Value);

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = NavigatableUI)
	float NavXValue();

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = NavigatableUI)
	float NavYValue();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = NavigatableUI)
	void OnNavChange(bool Reset = false);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = NavigatableUI)
	void OnConfirm();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = NavigatableUI)
	void OnCancel();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = NavigatableUI)
	void OnEnter();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = NavigatableUI)
	void OnExit();

	UFUNCTION(BlueprintCallable, Category = NavigatableUI)
	void ForceFocus();

	UFUNCTION(BlueprintCallable, Category = NavigatableUI)
	void ResetCurrentOption();

	UFUNCTION(BlueprintCallable, Category = NavigatableUI)
	void SetCurrentOption(int32 NewCurrentOption);

	UFUNCTION(BlueprintCallable, Category = NavigatableUI)
	void AddHudMessage(int32 ID, FString Message, EMessageType MessageType = EMessageType::E_NONE) const;

	UPROPERTY(Category = NavigatableUI, VisibleAnywhere, BlueprintReadWrite)
	TArray<TScriptInterface<IMenuOption>> MenuOptions;


protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UDefaultGameInstance* GameInstance = nullptr;

	UPROPERTY(Category = NavigatableUI, BlueprintReadOnly)
	UInputComponent* Input = nullptr;

	UPROPERTY(Category = NavigatableUI, VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentMenuOption = 0;

	UPROPERTY(Category = NavigatableUI, VisibleAnywhere, BlueprintReadWrite)
	int32 PreviousMenuOption = 0;

	UPROPERTY(Category = NavigatableUI, VisibleAnywhere, BlueprintReadOnly)
	float TimeChanged = 0.0f;

	UPROPERTY(Category = NavigatableUI, EditAnywhere, BlueprintReadWrite)
	float ScrollDelay = 0.2f;
	
	UPROPERTY(Category = NavigatableUI, EditAnywhere, BlueprintReadWrite)
	bool CheckTransparencyOnScroll = true;

	UPROPERTY(Category = NavigatableUI, EditAnywhere, BlueprintReadWrite)
	bool NavXInverted = false;

	UPROPERTY(Category = NavigatableUI, EditAnywhere, BlueprintReadWrite)
	bool NavYInverted = false;

	UPROPERTY(Category = NavigatableUI, EditAnywhere, BlueprintReadWrite)
	int32 NavXStepSize = 1;

	UPROPERTY(Category = NavigatableUI, EditAnywhere, BlueprintReadWrite)
	int32 NavYStepSize = 1;
};
