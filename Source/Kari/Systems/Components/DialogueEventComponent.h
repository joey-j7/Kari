#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Events/DialogueEvent.h"

#include "DialogueEventComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FDialogueEventDelegate_OnDialogueEventBroadcast, FDialogueEvent, bool);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogueEventDelegate_OnEnter, FDialogueEvent, DialogueEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogueEventDelegate_OnExit, FDialogueEvent, DialogueEvent);


UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class KARI_API UDialogueEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueEventComponent();

	UFUNCTION(BlueprintInternalUseOnly)
	void OnDialogueEvent(FDialogueEvent DialogueEvent, bool IsOnEnter);

	UPROPERTY(BlueprintAssignable, Category = "Dialogue Event")
	FDialogueEventDelegate_OnEnter OnEnter;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue Event")
	FDialogueEventDelegate_OnExit OnExit;

	UPROPERTY()
	FString EventName;

	static FDialogueEventDelegate_OnDialogueEventBroadcast OnDialogueEventDelegate;
};
