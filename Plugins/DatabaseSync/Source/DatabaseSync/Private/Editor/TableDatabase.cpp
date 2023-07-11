#include "TableDatabase.h"
#include "ModuleManager.h"

#if WITH_EDITOR
	#include "PropertyEditorModule.h"
#endif

TableDatabase* TableDatabase::Instance = nullptr;

TableDatabase::TableDatabase()
{
	IFileManager& FileManager = IFileManager::Get();

	// Get existing database files
	FString path = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	path.Append("Data/");

	TArray<FString> files;
	FileManager.FindFiles(files, *path, *FString("udat"));

	FString name, ext;

	for (int32_t i = 0; i < files.Num(); ++i)
	{
		FPaths::Split(files[i], path, name, ext);
		Binaries.Add(name, TableBinary(TCHAR_TO_UTF8(*name)));
	}

	// Get database version
	UpdateTime();
}

TableDatabase& TableDatabase::Get()
{
	if (!Instance)
		Instance = new TableDatabase();

	return *Instance;
}

const FTableData& TableDatabase::GetTable(const FString& tableName)
{
	FTableData* table = const_cast<FTableData*>(Data.Find(tableName));

	if (!table)
	{
		Data.Add(tableName, TableBinary::GetTable(tableName));
		table = const_cast<FTableData*>(Data.Find(tableName));

		ensure(table);
	}

	return *table;
}

const TMap<uint32, FTableRow>& TableDatabase::GetRows(const FString& tableName)
{
	return const_cast<FTableData&>(GetTable(tableName)).Rows;
}

const FTableRow& TableDatabase::GetRow(const FString& tableName, uint32 id)
{
	FTableData& table = const_cast<FTableData&>(GetTable(tableName));
	FTableRow* row = const_cast<FTableRow*>(table.Rows.Find(id));

	if (!row)
	{
		const FTableRow r = TableBinary::GetRow(tableName, id);
		table.Rows.Add(id, r);
		row = table.Rows.Find(id);
	}

	return *row;
}

const FTableField* TableDatabase::GetField(const FString& tableName, uint32 columnId, uint32 id)
{
	FTableRow& row = const_cast<FTableRow&>(GetRow(tableName, id));

	for (auto& field : row.Fields)
	{
		if (field.Key.ColumnID != columnId)
			continue;

		return &field.Value;
	}

	return nullptr;
}

const FTableField* TableDatabase::GetField(const FString& tableName, const FString& columnName, uint32 id)
{
	FTableRow& row = const_cast<FTableRow&>(GetRow(tableName, id));

	for (auto& field : row.Fields)
	{
		if (field.Key.ColumnName != columnName)
			continue;

		return &field.Value;
	}

	return nullptr;
}

TPair<uint32, const FTableRow*> TableDatabase::FindLink(const FString& tableName, const FString& columnName, uint32 id)
{
	const FTableData& table = GetTable(tableName);

	for (auto& row : table.Rows)
	{
		for (auto& field : row.Value.Fields)
		{
			if (!(field.Key == columnName))
				continue;

			if (field.Value.Size != 0)
			{
				double d = UINT_MAX;
				memcpy(&d, field.Value.Data.Get(), FMath::Min((size_t)sizeof(double), (size_t)field.Value.Size));

				if ((uint32)d == id)
					return TPair<uint32, const FTableRow*>(row.Key, &row.Value);
			}
		}
	}

	return TPair<uint32, const FTableRow*>(UINT_MAX, nullptr);
}

TArray<TPair<uint32, const FTableRow*>> TableDatabase::FindLinks(const FString& tableName, const FString& columnName, uint32 id)
{
	TArray<TPair<uint32, const FTableRow*>> Result;

	const FTableData& table = GetTable(tableName);

	for (auto& row : table.Rows)
	{
		for (auto& field : row.Value.Fields)
		{
			if (!(field.Key == columnName))
				continue;

			if (field.Value.Size != 0)
			{
				double d = UINT_MAX;
				memcpy(&d, field.Value.Data.Get(), FMath::Min((size_t)sizeof(double), (size_t)field.Value.Size));

				if ((uint32)d == id)
					Result.Add(TPair<uint32, const FTableRow*>(row.Key, &row.Value));
			}
		}
	}

	return Result;
}

const TMap<FString, TableBinary>& TableDatabase::GetBinaries() const
{
	return Binaries;
}

TableBinary& TableDatabase::GetBinary(const FString& tableName)
{
	TableBinary* binary = Binaries.Find(tableName);

	if (!binary)
	{
		TableBinary bin = TableBinary(TCHAR_TO_UTF8(*tableName));
		Binaries.Add(tableName, bin);

		binary = &Binaries[tableName];
	}

	return *binary;
}

void TableDatabase::RemoveCache()
{
	Data.Empty();
}

void TableDatabase::UpdateTime()
{
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/update.time");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*path);

	if (FileHandle)
	{
		FileHandle->Read((uint8*)&DatabaseVersion, sizeof(uint64));
		delete FileHandle;
	}
}

void TableDatabase::Refresh()
{
	for (auto& binary : Binaries)
	{
		binary.Value.Refresh();
	}

	RemoveCache();
	UpdateTime();

#if WITH_EDITOR
	// Update editor
	auto& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >("PropertyEditor");
	PropertyModule.NotifyCustomizationModuleChanged();
#endif
}
