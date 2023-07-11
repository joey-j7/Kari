#pragma once

#include "UDAT.h"

#include "TableField.h"
#include "TableRow.h"
#include "TableData.h"

class TableBinary
{
public:
	TableBinary(const std::string& sName);

	static const char m_Extension[5];

	// Gets a value from the binary file
	static FTableField GetValue(const FString& tableName, uint32_t columnID, uint32_t entityID);

	// Gets a row from the binary file
	static FTableRow GetRow(const FString& tableName, uint32_t entityID);

	// Gets entire table from the binary file
	static FTableData GetTable(const FString& tableName);

	// Finds all instances containing ID in column
	static TArray<FTableRow> FindRows(const FString& tableName, uint32_t columnID, uint32 findID);

	// Convert column name to ID
	uint32_t GetColumnID(const FString& columnName) const;

	// Convert column ID to name
	FString GetColumnName(uint32 columnId) const;

	// Import JSON to memory
	void Import(const TSharedPtr<FJsonObject>& entries);

	// Export memory to binary file
	void Export();

	// Read data from binary file to memory
	void Refresh();

	// Populate a list of entities, showing a specific column, used for dropdowns
	const TMap<uint32, FString>& Populate(const FString& columnToShow);
	TMap<uint32, FString> PopulateWithLink(const FString& columnToShow, const FString& linkedColumn, uint32 linkedId);

private:
	// Size followed by actual data
	TArray<FTableField> m_Data;

	// List of entity names
	TMap<uint32, FString> m_List;
	FString m_ShownColumn = "";

	bool m_bListUpdated = false;

	UDAT m_UDAT;
};
