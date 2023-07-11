#pragma once

#include "CoreMinimal.h"

#include "DialogueOptionEvent.generated.h"

USTRUCT(BlueprintType)
struct FDialogueOptionEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Option Event")
	FString EventName;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Option Event")
	int32 DialogueOptionID = UINT_MAX;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Option Event")
	int32 CharacterID = UINT_MAX;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Option Event")
	int32 Value = UINT_MAX;
};
