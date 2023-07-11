#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "TableWidget.generated.h"

class UStoryComponent;

/**
 * 
 */
UCLASS()
class DATABASESYNC_API UTableWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Table)
	const FString GetField(const FString& tableName, const FString& columnName, int32 id) const;
	
	UFUNCTION(BlueprintCallable, Category = Table)
	TArray<FString> GetRows(const FString& tableName, const FString& columnName) const;

protected:

};
