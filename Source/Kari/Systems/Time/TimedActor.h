#pragma once

#include "CoreMinimal.h"

#include "Systems/Interfaces/Timeable.h"
#include "GameFramework/Actor.h"

#include "TimeManager.h"

#include "TimedActor.generated.h"

UCLASS(BlueprintType)
class KARI_API ATimedActor : public AActor, public ITimeable
{
	GENERATED_BODY()

	friend class UTimeManager;

public:
	void BeginPlay() override;
	void BeginDestroy() override;

protected:
	UFUNCTION(BlueprintCallable)
	void UpdatePeriod();

	void OnPeriod(EPeriod Period, UObject* Caller) override;

	UDefaultGameInstance* GameInstance = nullptr;
	UTimeManager* TimeManager = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EPeriod> Period = EPeriod::E_AFTERNOON;
};