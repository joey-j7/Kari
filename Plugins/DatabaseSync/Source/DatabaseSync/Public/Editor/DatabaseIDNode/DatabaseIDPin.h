#pragma once
#include "CoreMinimal.h"
#include "DatabaseIDPin.generated.h"

USTRUCT(BlueprintType)
struct DATABASESYNC_API FDatabaseIDPin
{
	GENERATED_BODY()

public:
	FDatabaseIDPin() : TableName(""), ColumnName(""), ID(0) {}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TableName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString ColumnName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ID;
};