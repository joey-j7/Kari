#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Events/TimeEvent.h"

#include "TimeEventComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FTimeEventDelegate_OnTimeEventBroadcast, FTimeEvent, bool);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeEventDelegate_OnEnter, FTimeEvent, TimeEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeEventDelegate_OnExit, FTimeEvent, TimeEvent);


UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class KARI_API UTimeEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTimeEventComponent();

	UFUNCTION(BlueprintInternalUseOnly)
	void OnTimeEvent(FTimeEvent TimeEvent, bool IsOnEnter);

	UPROPERTY(BlueprintAssignable, Category = "Time Event")
	FTimeEventDelegate_OnEnter OnEnter;

	UPROPERTY(BlueprintAssignable, Category = "Time Event")
	FTimeEventDelegate_OnExit OnExit;

	UPROPERTY()
	FString EventName;

	static FTimeEventDelegate_OnTimeEventBroadcast OnTimeEventDelegate;
};
