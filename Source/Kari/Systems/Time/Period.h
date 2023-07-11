#pragma once

#include "Period.generated.h"

struct FTableRow;

UENUM(BlueprintType)
enum EPeriod
{
	E_NIGHT UMETA(DisplayName = "Night"),
	E_MORNING UMETA(DisplayName = "Morning"),
	E_AFTERNOON UMETA(DisplayName = "Afternoon"),
	E_EVENING UMETA(DisplayName = "Evening"),
	E_NUM UMETA(DisplayName = "Num"),
};

USTRUCT(BlueprintType)
struct FHourMinute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 Hour = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 Minute = 0;
};

USTRUCT(BlueprintType)
struct FTimedEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 ID = -INT_MAX;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 CharacterID = -INT_MAX;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 PeriodID = -INT_MAX;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 Value = -INT_MAX;

	static FTimedEvent ConvertRow(const FTableRow& Row, int32 ID);
};

USTRUCT(BlueprintType)
struct FPeriod
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TEnumAsByte<EPeriod> Enum = EPeriod::E_NIGHT;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 ID = -INT_MAX;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	FString Name = "";

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 Hour = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 Minute = 0;

	static FPeriod ConvertRow(const FTableRow& Row, int32 ID);
};