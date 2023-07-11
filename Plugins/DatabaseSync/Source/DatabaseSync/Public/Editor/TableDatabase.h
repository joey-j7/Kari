#pragma once

#include "CoreMinimal.h"
#include "TableBinary.h"

#include "TableData.h"

class DATABASESYNC_API TableDatabase
{
public:
	TableDatabase();

	static TableDatabase& Get();

	// Gets an entire table from memory
	const FTableData& GetTable(const FString& tableName);

	// Gets a row from memory
	const TMap<uint32, FTableRow>& GetRows(const FString& tableName);
	const FTableRow& GetRow(const FString& tableName, uint32 id);

	// Gets a field from binary if non-existent, otherwise gets it from memory
	const FTableField* GetField(const FString& tableName, uint32 columnId, uint32 id);
	const FTableField* GetField(const FString& tableName, const FString& columnName, uint32 id);

	TPair<uint32, const FTableRow*> FindLink(const FString& tableName, const FString& columnName, uint32 id);
	TArray<TPair<uint32, const FTableRow*>> FindLinks(const FString& tableName, const FString& columnName, uint32 id);

	// Get the timestamp of the last synced database version
	uint64 GetDatabaseVersion() const { return DatabaseVersion; }

	// Get all binary files, one for each table
	const TMap<FString, TableBinary>& GetBinaries() const;

	// Gets a binary file from table name
	TableBinary& GetBinary(const FString& tableName);

	// Deletes all the data cached in memory
	void RemoveCache();

	// Reads from all the binaries, updates data and updates the editor
	void Refresh();

private:
	void UpdateTime();
	
	static TableDatabase* Instance;
	uint64 DatabaseVersion = 0;

	TMap<FString, TableBinary> Binaries;

	// All the data (sorted per table) the application needs for reading
	TMap<FString, FTableData> Data;
};
