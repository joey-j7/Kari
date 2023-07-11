#pragma once

#include "Queue.h"
#include "Blueprint/UserWidget.h"
#include "General/DefaultGameInstance.h"
#include "Systems/Quests/Quest.h"
#include "UI/HUD/Log/HUDLogWidget.h"
#include "ActionLogWidget.generated.h"

class UInventory;
class UQuestLog;
class ADefaultPlayer;

#define HUD_MESSAGE_FADE_TIME .2f

UENUM(BlueprintType)
enum class EMessageType : uint8
{
	E_NONE = 0 UMETA(DisplayName = "None"),
    E_ITEM_ADD = 1 UMETA(DisplayName = "Item Add Message"),
    E_ITEM_DROP = 2 UMETA(DisplayName = "Item Drop Message"),
	E_TASK = 3 UMETA(DisplayName = "Task Message"),
    E_QUEST = 4 UMETA(DisplayName = "Quest Message"),
    E_UNDEFINED = 5 UMETA(DisplayName = "UNDEFINED")
};

USTRUCT(BlueprintType)
struct FActionLogMessage : public FHUDMessage
{
	GENERATED_BODY()

    FActionLogMessage() {}
	/**
	 * @brief - Create an message for the action log
	 * @param MsgID - referring to the item ID such as taskId, questId, dialogueId etc.
	 * @param Msg - Message we should output.
	 * @param MsgType - Specify the type of the action log message
	 * @param MsgStack - Specify the counter for the items.
	 * @param Time - Time how long it should be shown
	 */
	FActionLogMessage(int32 MsgID, const FString& Msg, EMessageType MsgType, int32 MsgStack = 1, float Time = 3.f): FHUDMessage(Msg, Time), ID(MsgID), Stack(MsgStack), MessageType(MsgType) {}

	UPROPERTY(BlueprintReadOnly, Category = "Action Log Message")
	int32 ID = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Action Log Message")
	int32 Stack = 1;
	
	UPROPERTY(BlueprintReadOnly, Category = "Action Log Message")
    EMessageType MessageType = EMessageType::E_NONE;
};

UCLASS()
class KARI_API UActionLogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UActionLogWidget(const FObjectInitializer& ObjectInitializer);

	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "HUD Log Widget")
	void AddMessage(int32 ID, const FString& Text, EMessageType msgType = EMessageType::E_NONE, int32 MsgStack = 1, float Time = 3.f);

	UFUNCTION(BlueprintNativeEvent)
	void ShowPopup(const FActionLogMessage& Message);

	UFUNCTION(BlueprintCallable)
	void HidePopup();

	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionIDChange(int32 InteractionID);

	UFUNCTION(BlueprintCallable)
	bool IsShowing() const { return Showing; }
	
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void IsInCameraShot(bool InCameraShot) { IsInCameraShotTrigger = InCameraShot; }
	
protected:
	TQueue<FActionLogMessage> Messages;

	UPROPERTY(BlueprintReadWrite)
	bool Showing = false;
	
	UPROPERTY(BlueprintReadWrite)
	ADefaultPlayer* PlayerReference = nullptr;

	int32 CurrentID = -INT_MAX;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FActionLogMessage CurrentMessage = {};
	
	UQuestLog* QuestLog = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool IsInCameraShotTrigger = false;
};
