#pragma once

#include "Task.generated.h"

struct FTableRow;
class UDefaultEvent;

UENUM(BlueprintType)
enum ETaskType
{
	E_COLLECT UMETA(DisplayName = "Collect"),
	E_DEFEAT UMETA(DisplayName = "Defeat"),
	E_TALK UMETA(DisplayName = "Talk"),
	E_INTERACT UMETA(DisplayName = "Interact"),
	E_DEFEND UMETA(DisplayName = "Defend")
};

USTRUCT(BlueprintType)
struct FTask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	int32 ID = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	FString Description = "";

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	int32 RequiredCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	TEnumAsByte<ETaskType> Type = E_COLLECT;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	int32 NextID = -1;

	// ID referring to the Non Playable Character table
	// UPROPERTY(BlueprintReadWrite, Category = "Task")
	uint32 NPC = UINT_MAX;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	int32 ParentID = -1;
	
	UPROPERTY(BlueprintReadWrite, Category = "Task")
	int32 ChildID = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	bool HiddenAtFirst = false;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	bool Active = false;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	bool Completed = false;

	// Freezes task completion checks if task is already completed
	UPROPERTY(BlueprintReadWrite, Category = "Task")
	bool Frozen = false;

	// Allows to revert revision ID's if the task is reverted
	UPROPERTY(BlueprintReadOnly, Category = "Task")
	TMap<int32, int32> RevertIDs;

	static FTask ConvertRow(const FTableRow& Row, int32 ID);

	void SaveLoad(FArchive &Ar);
};