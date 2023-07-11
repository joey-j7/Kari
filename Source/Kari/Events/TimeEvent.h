#pragma once
#include "CoreMinimal.h"

#include "TimeEvent.generated.h"

USTRUCT(BlueprintType)
struct FTimeEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Time Event")
	FString EventName;
	
	UPROPERTY(BlueprintReadOnly, Category = "Time Event")
	int32 PeriodID = UINT_MAX;

	UPROPERTY(BlueprintReadOnly, Category = "Time Event")
	int32 CharacterID = UINT_MAX;
	
	UPROPERTY(BlueprintReadOnly, Category = "Time Event")
	int32 Value = UINT_MAX;
};