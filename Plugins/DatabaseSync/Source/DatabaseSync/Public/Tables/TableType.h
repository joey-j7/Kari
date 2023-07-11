#pragma once

#include "CoreMinimal.h"

#include "UObject/Class.h"
#include "Templates/SubclassOf.h"

class TableType : public UClass
{
public:
	TableType(
		const FString& tableName,
		const FString& dropdownColumn,
		const TMap<FString, TSubclassOf<TableType>>& links,
		const TMap<FString, TSubclassOf<TableType>>& externalLinks
	);

	const FString& GetTableName() const { return TableName; }
	const FString& GetDropdownColumn() const { return DropdownColumn; }

	const TMap<FString, TSubclassOf<TableType>>& GetLinks() const { return Links; };
	const TMap<FString, TSubclassOf<TableType>>& GetExternalLinks() const { return ExternalLinks; };

protected:
	FString TableName = "";
	FString DropdownColumn = "";

	// Links own column to another table
	TMap<FString, TSubclassOf<TableType>> Links;

	// Links another table's column to this table
	TMap<FString, TSubclassOf<TableType>> ExternalLinks;
};
