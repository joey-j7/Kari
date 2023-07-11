#pragma once
#include "CoreMinimal.h"

#include "TaskEvent.generated.h"

USTRUCT(BlueprintType)
struct FTaskEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Task Event")
	FString EventName;
	
	UPROPERTY(BlueprintReadOnly, Category = "Task Event")
	int32 TaskID = UINT_MAX;

	UPROPERTY(BlueprintReadOnly, Category = "Task Event")
	int32 CharacterID = UINT_MAX;
	
	UPROPERTY(BlueprintReadOnly, Category = "Task Event")
	int32 Value = UINT_MAX;
};