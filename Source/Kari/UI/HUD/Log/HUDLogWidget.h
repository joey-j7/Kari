

#pragma once

#include "Blueprint/UserWidget.h"
#include "Systems/Quests/Quest.h"
#include "HUDLogWidget.generated.h"

class UInventory;
class UQuestLog;

#define HUD_MESSAGE_FADE_TIME .2f

USTRUCT(BlueprintType)
struct FHUDMessage
{
	GENERATED_BODY()

	FHUDMessage() {}
	FHUDMessage(const FString& Msg, float Time = 3.f) : Message(Msg), DisplayTime(Time), Timer(Time) {}

	UPROPERTY(BlueprintReadOnly, Category = "HUD Message")
	FString Message = "Empty Message";

	UPROPERTY(BlueprintReadOnly, Category = "HUD Message")
	float DisplayTime = 3.f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD Message")
	float Timer = 3.f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD Message")
	float FadeTimer = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD Message")
	float FadeTimerNorm = 0.f;
};

USTRUCT(BlueprintType)
struct FTaskLog
{
	GENERATED_BODY()

	FTaskLog() {}
	FTaskLog(const FString& Msg, int32 Cur = 0, int32 Req = 0, bool Comp = false) : Message(Msg), Current(Cur), Required(Req), Completed(Comp) {}

	UPROPERTY(BlueprintReadOnly, Category = "HUD Task")
	FString Message = "Empty Task";

	UPROPERTY(BlueprintReadOnly, Category = "HUD Task")
	int32 Current = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HUD Task")
	int32 Required = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HUD Task")
	bool Completed = false;
};

UCLASS()
class KARI_API UHUDLogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UHUDLogWidget(const FObjectInitializer& ObjectInitializer);

	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "HUD Log Widget")
	void AddMessage(const FString& Text, float Time = 3.f);

	UFUNCTION(BlueprintCallable, Category = "HUD Log Widget")
	const TArray<FHUDMessage>& GetMessages() const { return Messages; };
	
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "HUD Log Widget")
	void OnQuestChange(const FQuest& Quest);

	TArray<FHUDMessage> Messages;

	UQuestLog* QuestLog = nullptr;
};
