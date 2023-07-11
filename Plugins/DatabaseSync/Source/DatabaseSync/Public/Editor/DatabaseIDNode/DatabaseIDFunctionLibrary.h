#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DatabaseIDPin.h"
#include "DatabaseIDFunctionLibrary.generated.h"

UCLASS()
class DATABASESYNC_API UDatabaseIDFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UDatabaseIDFunctionLibrary() = default;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database", meta = (DisplayName = "Get Database ID"))
	static int32 GetDatabaseID(FDatabaseIDPin DatabaseEntry) { return DatabaseEntry.ID; }
};