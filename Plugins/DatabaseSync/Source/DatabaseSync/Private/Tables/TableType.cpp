#include "TableType.h"

TableType::TableType(
	const FString& tableName,
	const FString& dropdownColumn,
	const TMap<FString, TSubclassOf<TableType>>& links,
	const TMap<FString, TSubclassOf<TableType>>& externalLinks
)
{
	TableName = tableName;
	DropdownColumn = dropdownColumn;
	Links = links;
	ExternalLinks = externalLinks;
}
