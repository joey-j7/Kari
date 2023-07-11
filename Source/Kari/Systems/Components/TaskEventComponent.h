#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Events/TaskEvent.h"

#include "TaskEventComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FTaskEventDelegate_OnTaskEventBroadcast, FTaskEvent, bool);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTaskEventDelegate_OnEnter, FTaskEvent, TaskEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTaskEventDelegate_OnExit, FTaskEvent, TaskEvent);


UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class KARI_API UTaskEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTaskEventComponent();

	UFUNCTION(BlueprintInternalUseOnly)
	void OnTaskEvent(FTaskEvent TaskEvent, bool IsOnEnter);

	UPROPERTY(BlueprintAssignable, Category = "Task Event")
	FTaskEventDelegate_OnEnter OnEnter;

	UPROPERTY(BlueprintAssignable, Category = "Task Event")
	FTaskEventDelegate_OnExit OnExit;

	UPROPERTY()
	FString EventName;

	static FTaskEventDelegate_OnTaskEventBroadcast OnTaskEventDelegate;
};
