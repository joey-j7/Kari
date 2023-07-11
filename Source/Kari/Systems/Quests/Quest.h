#pragma once

#include "Task.h"
#include "Quest.generated.h"

struct FTableRow;

UENUM(BlueprintType)
enum EQuestType
{
	E_MAIN UMETA(DisplayName = "Main"),
	E_SUB UMETA(DisplayName = "Sub")
};

USTRUCT(BlueprintType)
struct FQuest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 ID = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FString Title = "";

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FText Description = FText::GetEmpty();

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 MoneyReward = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TEnumAsByte<EQuestType> Type = E_SUB;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TArray<FTask> Tasks;
	
	UPROPERTY(BlueprintReadWrite, Category = "Task")
	bool Completed = false;

	static FQuest ConvertRow(const FTableRow& Row, int32 ID);

	void Save(FArchive &Ar);
	void Load(FArchive &Ar);
};