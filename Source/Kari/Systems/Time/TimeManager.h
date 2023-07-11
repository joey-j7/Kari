#pragma once

#include "CoreMinimal.h"
#include "Period.h"

#include "Systems/Interfaces/Timeable.h"
#include "Systems/Components/CharacterComponent.h"

#include "TimeManager.generated.h"

#define SECONDS_PER_DAY 86400

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeManagerDelegate_OnPeriodChanged, EPeriod, Period);

class ATimedActor;

UCLASS(BlueprintType)
class KARI_API UTimeManager : public UTableComponent
{
	GENERATED_BODY()

public:
	UTimeManager();

	void Init();
	void Update(float DeltaTime);

	void Add(ATimedActor* Actor);
	void Remove(ATimedActor* Actor);

	UFUNCTION(BlueprintCallable)
	void UseRealTime(bool Use);

	UFUNCTION(BlueprintCallable)
	float GetNormalizedDayTime() const;

	UFUNCTION(BlueprintCallable)
	float GetNormalizedPeriodTime() const;

	UFUNCTION(BlueprintCallable)
	void SetTimeMultiplier(float S);

	UFUNCTION(BlueprintCallable)
	void SetTimeSpeed(float S);

	UFUNCTION(BlueprintCallable)
	FString GetTimeString(bool AddSeconds = false) const;

	/**
	 * @brief - Set time of the day in hours/minutes/seconds
	 * @param Hours,
	 * @param Minutes (Optional)
	 * @param Seconds (Optional)
	 */ 
	UFUNCTION(BlueprintCallable)
	void SetTime(int32 Hours, int32 Minutes = 0, int32 Seconds = 0);

	/**
	 * @brief - Toggle the time of the day
	 * @param Period - choose period of the day (Optional).
	 * If Enum then period will be overriden by the current period. See @function GetPeriod()
	 */
	UFUNCTION(BlueprintCallable)
	void ToggleTimeOfDay(EPeriod Period = E_NUM);

	UFUNCTION(BlueprintCallable)
	FString GetDateString() const;

	UFUNCTION(BlueprintCallable)
	const FDateTime& GetDate() const;

	UFUNCTION(BlueprintCallable)
	EPeriod GetPeriod() const { return Period; }

	UPROPERTY(BlueprintAssignable)
	FTimeManagerDelegate_OnPeriodChanged OnPeriodChanged;

protected:
	void UpdateEvents();
	void UpdatePeriod();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnMorning();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnAfternoon();

	UFUNCTION(BlueprintImplementableEvent)
	void OnEvening();

	UFUNCTION(BlueprintImplementableEvent)
	void OnNight();

	EPeriod Period = E_NIGHT;
	FDateTime Date;

	TArray<ATimedActor*> Timeables;

	TMap<EPeriod, FPeriod> Periods;
	TMap<EPeriod, TArray<FTimedEvent>> PeriodEvents;

	bool FirstFrame = true;

	uint64 CurrentDay = 0;
	double CurrentTime = 0;

	float Speed = 1.f;
	float Multiplier = 1.f;

	static float SecondsToDayTime;

	bool RealTimeSync = false;
};