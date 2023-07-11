#pragma once
#include "CoreMinimal.h"

#include "DialogueEvent.generated.h"

USTRUCT(BlueprintType)
struct FDialogueEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Event")
	FString EventName;
	
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Event")
	int32 DialogueID = UINT_MAX;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Event")
	int32 CharacterID = UINT_MAX;
	
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Event")
	int32 Value = UINT_MAX;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Event")
	int32 SecondaryValue = UINT_MAX;
};