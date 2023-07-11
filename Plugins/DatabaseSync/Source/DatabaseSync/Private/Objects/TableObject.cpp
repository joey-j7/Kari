#include "TableObject.h"
#include "TableDatabase.h"

const FTableField* UTableObject::GetField(const FString& tableName, const FString& columnName, uint32 id) const
{
	return TableDatabase::Get().GetField(tableName, columnName, id);
}

const FTableRow& UTableObject::GetRow(const FString& tableName, uint32 id) const
{
	return TableDatabase::Get().GetRow(tableName, id);
}

const FTableData& UTableObject::GetTable(const FString& tableName) const
{
	return TableDatabase::Get().GetTable(tableName);
}

TPair<uint32, const FTableRow*> UTableObject::FindLink(const FString& tableName, const FString& columnName, uint32 id) const
{
	return TableDatabase::Get().FindLink(tableName, columnName, id);
}

TArray<TPair<uint32, const FTableRow*>> UTableObject::FindLinks(const FString& tableName, const FString& columnName, uint32 id) const
{
	return TableDatabase::Get().FindLinks(tableName, columnName, id);
}