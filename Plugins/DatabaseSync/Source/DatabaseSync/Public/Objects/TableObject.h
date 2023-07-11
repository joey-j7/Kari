#pragma once

#include "CoreMinimal.h"

#include "TableField.h"
#include "TableData.h"

#include "TableObject.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATABASESYNC_API UTableObject : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTableObject() {};

	uint32 GetTableID() const { return ID; }
	void SetTableID(uint32 id) { ID = id; }

	const FString& GetTableName() const { return Name; }

	const FTableField* GetField(const FString& tableName, const FString& columnName, uint32 id) const;
	const FTableRow& GetRow(const FString& tableName, uint32 id) const;
	const FTableData& GetTable(const FString& tableName) const;

	TPair<uint32, const FTableRow*> FindLink(const FString& tableName, const FString& columnName, uint32 id) const;
	TArray<TPair<uint32, const FTableRow*>> FindLinks(const FString& tableName, const FString& columnName, uint32 id) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Table Properties")
	uint32 ID = UINT_MAX;

	UPROPERTY(VisibleAnywhere, Category = "Table Properties")
	FString Name = "";
};
