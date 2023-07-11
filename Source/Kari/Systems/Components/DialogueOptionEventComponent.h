#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Events/DialogueOptionEvent.h"

#include "DialogueOptionEventComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDialogueEventDelegate_OnDialogueOptionEventBroadcast, FDialogueOptionEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogueOptionEventDelegate_OnClicked, FDialogueOptionEvent, DialogueOptionEvent);

UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class KARI_API UDialogueOptionEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueOptionEventComponent();

	UFUNCTION(BlueprintInternalUseOnly)
	void OnDialogueOptionEvent(FDialogueOptionEvent DialogueOptionEvent);

	UPROPERTY(BlueprintAssignable, Category = "Dialogue Option Event")
	FDialogueOptionEventDelegate_OnClicked OnClick;

	/* TODO Events that might be needed in the future */
	 
	// UPROPERTY(BlueprintAssignable, Category = "Dialogue Event")
	// FDialogueOptionEventDelegate_OnEnter OnHovered;
	
	// UPROPERTY(BlueprintAssignable, Category = "Dialogue Event")
	// FDialogueOptionEventDelegate_OnExit OnExit;

	UPROPERTY()
	FString EventName;

	static FDialogueEventDelegate_OnDialogueOptionEventBroadcast OnDialogueOptionEventDelegate;
};
