#include "TableBinary.h"
#include "PlatformFilemanager.h"

#include "Paths.h"

#include "MessageDialog.h"
#include "Internationalization.h"

#include "SNotificationList.h"
#include "NotificationManager.h"
#include "FileManager.h"

#include <algorithm>
#include <ctime>
#include "TableDatabase.h"

#define LOCTEXT_NAMESPACE "TableBinary"

const char TableBinary::m_Extension[5] = { 'u', 'd', 'a', 't', '\0' };

#define ADD_NOTIFICATION FNotificationInfo Info(DialogText); \
Info.FadeInDuration = 0.1f; \
Info.FadeOutDuration = 0.5f; \
Info.ExpireDuration = 5.0f; \
Info.bUseThrobber = false; \
Info.bUseSuccessFailIcons = false; \
Info.bUseLargeFont = false; \
Info.bFireAndForget = true; \
Info.bAllowThrottleWhenFrameRateIsLow = false; \
auto Notification = FSlateNotificationManager::Get().AddNotification(Info); \

TableBinary::TableBinary(const std::string& sName)
{
	for (uint32 i = 0; i < FMath::Min((size_t)32, sName.size()); ++i)
	{
		m_UDAT.Name[i] = sName[i];
	}
	
	// Retrieve data from existing file, if it exists
	Refresh();
}

FTableField TableBinary::GetValue(const FString& tableName, uint32_t columnID, uint32_t entityID)
{
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/");
	path.Append(tableName);
	path.AppendChar('.');
	path.Append(FString(&m_Extension[0]));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*path);

	// Do nothing when file cannot be opened
	if (!FileHandle)
	{
		return {};
	}

	// Read and check header
	char MagicID[4];
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	// Invalid file
	if (strncmp(MagicID, "UDAT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return {};
	}

	uint32_t Size;
	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	FileHandle->Seek(Size - sizeof(uint32_t) * 2);

	uint32_t ColumnCount = 0;
	uint32_t EntityCount = 0;

	FileHandle->Read((uint8_t*)&ColumnCount, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&EntityCount, sizeof(uint32_t));

	// Invalid ID
	if (columnID >= ColumnCount || entityID >= EntityCount)
	{
		check(false);
		delete FileHandle;

		return {};
	}

	/* CSCT */
	uint32_t Position = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "CSCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return {};
	}

	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	FileHandle->Seek(Position + Size);

	/* DSCT */
	Position = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "DSCT", 4) != 0)
	{
		check(false);
		delete FileHandle;
		
		return {};
	}

	FileHandle->Seek(Position + 4 + entityID * sizeof(uint32_t));

	uint32_t Pointer;
	FileHandle->Read((uint8_t*)&Pointer, sizeof(uint32_t));

	FileHandle->Seek(Pointer);

	/* ESCT */
	Position = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "ESCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return {};
	}

	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));
	FileHandle->Seek(Position + 8 + columnID * sizeof(uint32_t) * 2);
	FileHandle->Read((uint8_t*)&Pointer, sizeof(uint32_t));

	FTableField Entry;
	FileHandle->Read((uint8_t*)&Entry.Size, sizeof(uint32_t));

	FileHandle->Seek(Pointer);
	Entry.Data = MakeShareable(new uint8_t[Entry.Size]);

	FileHandle->Read(Entry.Data.Get(), Entry.Size);
	delete FileHandle;

	return Entry;
}

FTableRow TableBinary::GetRow(const FString& tableName, uint32_t entityID)
{
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/");
	path.Append(tableName);
	path.AppendChar('.');
	path.Append(FString(&m_Extension[0]));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*path);

	// Do nothing when file cannot be opened
	if (!FileHandle)
	{
		return FTableRow();
	}

	// Read and check header
	char MagicID[4];
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	// Invalid file
	if (strncmp(MagicID, "UDAT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return FTableRow();
	}

	uint32_t Size;
	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	FileHandle->Seek(Size - sizeof(uint32_t) * 2);

	uint32_t ColumnCount = 0;
	uint32_t EntityCount = 0;

	FileHandle->Read((uint8_t*)&ColumnCount, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&EntityCount, sizeof(uint32_t));

	// Invalid ID
	if (entityID < 0 || entityID >= EntityCount)
	{
		check(false);
		delete FileHandle;

		return FTableRow();
	}

	/* CSCT */
	uint32_t Position = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "CSCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return FTableRow();
	}

	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	// Read data from pointers
	FTableRow Result;

	TArray<uint32_t> DataSizes;
	DataSizes.SetNum(ColumnCount);

	TArray<uint32_t> Pointers;
	Pointers.SetNumZeroed(ColumnCount);

	TArray<std::string> ColumnNames;
	ColumnNames.SetNum(ColumnCount);

	for (uint32_t i = 0; i < ColumnCount; ++i)
	{
		FileHandle->Seek(Position + 8 + i * sizeof(uint32_t));
		FileHandle->Read((uint8_t*)&Pointers[i], sizeof(uint32_t));

		// Take data chunk size
		if (i == ColumnCount - 1)
		{
			DataSizes[i] = (Position + Size) - Pointers[i] - 1;
		}
		// Use next pointer to determine size
		else
		{
			uint32_t p = 0;

			for (uint32_t k = i + 1; k < ColumnCount; ++k)
			{
				uint32 NextPointer = 0;
				FileHandle->Read((uint8_t*)&NextPointer, sizeof(uint32_t));

				if (NextPointer)
				{
					p = NextPointer;
					break;
				}
			}

			DataSizes[i] = p - Pointers[i] - 1;
		}
	}

	FileHandle->Seek(Pointers[0]);

	for (uint32_t i = 0; i < ColumnCount; ++i)
	{
		uint8_t t;
		FileHandle->Read(&t, sizeof(uint8_t));

		ColumnNames[i].resize(DataSizes[i] + 1);
		FileHandle->Read((uint8_t*)ColumnNames[i].c_str(), DataSizes[i]);
	}

	FileHandle->Seek(Position + Size);

	/* DSCT */
	Position = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "DSCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return FTableRow();
	}

	FileHandle->Seek(Position + 4 + entityID * sizeof(uint32_t));

	uint32_t Pointer;
	FileHandle->Read((uint8_t*)&Pointer, sizeof(uint32_t));

	if (Pointer == 0)
	{
		delete FileHandle;

		return FTableRow();
	}

	FileHandle->Seek(Pointer);

	/* ESCT */
	Position = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "ESCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return FTableRow();
	}

	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	TArray<FTableField> fields;
	fields.SetNum(ColumnCount);

	for (uint32_t i = 0; i < ColumnCount; ++i)
	{
		FileHandle->Read((uint8_t*)&Pointer, sizeof(uint32_t));
		FileHandle->Read((uint8_t*)&fields[i].Size, sizeof(uint32_t));
	}

	for (uint32_t i = 0; i < ColumnCount; ++i)
	{
		if (fields[i].Size == 0)
			continue;

		fields[i].Data = MakeShareable(new uint8_t[fields[i].Size]);
		FileHandle->Read(fields[i].Data.Get(), fields[i].Size);

		Result.Fields.Add(
			{ UTF8_TO_TCHAR(ColumnNames[i].c_str()), i },
			fields[i]
		);
	}

	delete FileHandle;
	return Result;
}

FTableData TableBinary::GetTable(const FString& tableName)
{
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/");
	path.Append(tableName);
	path.AppendChar('.');
	path.Append(FString(&m_Extension[0]));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*path);

	// Do nothing when file cannot be opened
	if (!FileHandle)
	{
		return FTableData();
	}

	// Read and check header
	char MagicID[4];
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	// Invalid file
	if (strncmp(MagicID, "UDAT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return FTableData();
	}

	uint32_t Size;
	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	FileHandle->Seek(Size - sizeof(uint32_t) * 2);

	uint32_t ColumnCount = 0;
	uint32_t EntityCount = 0;

	FileHandle->Read((uint8_t*)&ColumnCount, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&EntityCount, sizeof(uint32_t));

	/* CSCT */
	uint32_t Position = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "CSCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return FTableData();
	}

	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	// Read data from pointers
	TArray<uint32_t> DataSizes;
	DataSizes.SetNum(ColumnCount);

	TArray<uint32_t> Pointers;
	Pointers.SetNumZeroed(ColumnCount);

	TArray<std::string> ColumnNames;
	ColumnNames.SetNum(ColumnCount);

	for (uint32_t i = 0; i < ColumnCount; ++i)
	{
		FileHandle->Seek(Position + 8 + i * sizeof(uint32_t));
		FileHandle->Read((uint8_t*)&Pointers[i], sizeof(uint32_t));

		// Take data chunk size
		if (i == ColumnCount - 1)
		{
			DataSizes[i] = (Position + Size) - Pointers[i] - 1;
		}
		// Use next pointer to determine size
		else
		{
			uint32_t p = 0;

			for (uint32_t k = i + 1; k < ColumnCount; ++k)
			{
				uint32 NextPointer = 0;
				FileHandle->Read((uint8_t*)&NextPointer, sizeof(uint32_t));

				if (NextPointer)
				{
					p = NextPointer;
					break;
				}
			}

			DataSizes[i] = p - Pointers[i] - 1;
		}
	}

	FileHandle->Seek(Pointers[0]);

	for (uint32_t i = 0; i < ColumnCount; ++i)
	{
		uint8_t t;
		FileHandle->Read(&t, sizeof(uint8_t));

		ColumnNames[i].resize(DataSizes[i] + 1);
		FileHandle->Read((uint8_t*)ColumnNames[i].c_str(), DataSizes[i]);
	}

	FileHandle->Seek(Position + Size);

	/* DSCT */
	uint32 DSCTPosition = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "DSCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return FTableData();
	}

	FTableData Result;

	for (uint32_t i = 0; i < EntityCount; ++i)
	{
		FTableRow row;

		FileHandle->Seek(DSCTPosition + 4 + i * sizeof(uint32_t));

		uint32_t Pointer;
		FileHandle->Read((uint8_t*)&Pointer, sizeof(uint32_t));

		// Deleted entry
		if (Pointer == 0)
			continue;

		FileHandle->Seek(Pointer);

		/* ESCT */
		Position = FileHandle->Tell();
		FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

		if (strncmp(MagicID, "ESCT", 4) != 0)
		{
			check(false);
			delete FileHandle;

			return FTableData();
		}

		FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

		TArray<FTableField> fields;
		fields.SetNum(ColumnCount);

		for (uint32_t j = 0; j < ColumnCount; ++j)
		{
			FileHandle->Read((uint8_t*)&Pointer, sizeof(uint32_t));
			FileHandle->Read((uint8_t*)&fields[j].Size, sizeof(uint32_t));
		}

		for (uint32_t j = 0; j < ColumnCount; ++j)
		{
			if (fields[j].Size == 0)
				continue;

			fields[j].Data = MakeShareable(new uint8_t[fields[j].Size]);
			FileHandle->Read(fields[j].Data.Get(), fields[j].Size);

			row.Fields.Add(
				{ UTF8_TO_TCHAR(ColumnNames[j].c_str()), j },
				fields[j]
			);
		}

		Result.Rows.Add(i, row);
	}

	delete FileHandle;
	return Result;
}

TArray<FTableRow> TableBinary::FindRows(const FString& tableName, uint32_t columnID, uint32 findID)
{
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/");
	path.Append(tableName);
	path.AppendChar('.');
	path.Append(FString(&m_Extension[0]));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*path);

	// Do nothing when file cannot be opened
	if (!FileHandle)
	{
		return {};
	}

	// Read and check header
	char MagicID[4];
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	// Invalid file
	if (strncmp(MagicID, "UDAT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return {};
	}

	uint32_t Size;
	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	FileHandle->Seek(Size - sizeof(uint32_t) * 2);

	uint32_t ColumnCount = 0;
	uint32_t EntityCount = 0;

	FileHandle->Read((uint8_t*)&ColumnCount, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&EntityCount, sizeof(uint32_t));

	// Invalid ID
	if (columnID >= ColumnCount)
	{
		check(false);
		delete FileHandle;

		return {};
	}

	/* CSCT */
	uint32_t Position = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "CSCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return {};
	}

	FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

	// Read data from pointers
	TArray<uint32_t> DataSizes;
	DataSizes.SetNum(ColumnCount);

	TArray<uint32_t> Pointers;
	Pointers.SetNumZeroed(ColumnCount);

	TArray<std::string> ColumnNames;
	ColumnNames.SetNum(ColumnCount);

	for (uint32_t i = 0; i < ColumnCount; ++i)
	{
		FileHandle->Seek(Position + 8 + i * sizeof(uint32_t));
		FileHandle->Read((uint8_t*)&Pointers[i], sizeof(uint32_t));

		// Take data chunk size
		if (i == ColumnCount - 1)
		{
			DataSizes[i] = (Position + Size) - Pointers[i] - 1;
		}
		// Use next pointer to determine size
		else
		{
			uint32_t p = 0;

			for (uint32_t k = i + 1; k < ColumnCount; ++k)
			{
				uint32 NextPointer = 0;
				FileHandle->Read((uint8_t*)&NextPointer, sizeof(uint32_t));

				if (NextPointer)
				{
					p = NextPointer;
					break;
				}
			}

			DataSizes[i] = p - Pointers[i] - 1;
		}
	}

	FileHandle->Seek(Pointers[0]);

	for (uint32_t i = 0; i < ColumnCount; ++i)
	{
		uint8_t t;
		FileHandle->Read(&t, sizeof(uint8_t));

		ColumnNames[i].resize(DataSizes[i] + 1);
		FileHandle->Read((uint8_t*)ColumnNames[i].c_str(), DataSizes[i]);
	}

	FileHandle->Seek(Position + Size);

	/* DSCT */
	uint32_t DSCTPosition = FileHandle->Tell();
	FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

	if (strncmp(MagicID, "DSCT", 4) != 0)
	{
		check(false);
		delete FileHandle;

		return {};
	}

	// Get entities
	TArray<FTableRow> Result;

	for (uint32_t i = 0; i < EntityCount; ++i)
	{
		FileHandle->Seek(DSCTPosition + 4 + i * sizeof(uint32_t));

		uint32_t Pointer;
		FileHandle->Read((uint8_t*)&Pointer, sizeof(uint32_t));

		FileHandle->Seek(Pointer);

		/* ESCT */
		Position = Pointer;
		FileHandle->Read((uint8_t*)&MagicID, sizeof(uint32_t));

		if (strncmp(MagicID, "ESCT", 4) != 0)
		{
			check(false);
			delete FileHandle;

			return {};
		}

		FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));
		FileHandle->Seek(Position + 8 + columnID * sizeof(uint32_t) * 2);

		FileHandle->Read((uint8_t*)&Pointer, sizeof(uint32_t));
		FileHandle->Read((uint8_t*)&Size, sizeof(uint32_t));

		if (Size == 0)
			continue;

		FileHandle->Seek(Pointer);

		uint8_t* Data = new uint8_t[Size];
		FileHandle->Read(Data, Size);

		double d = findID;

		// Check if data matches input
		if (memcmp(Data, &d, sizeof(double)) == 0)
		{
			FTableRow row;

			FileHandle->Seek(Position + 8);

			TArray<uint32_t> Ptrs;
			TArray<uint32_t> Sizes;

			Ptrs.SetNum(ColumnCount);
			Sizes.SetNum(ColumnCount);

			for (uint32_t j = 0; j < ColumnCount; ++j)
			{
				FileHandle->Read((uint8_t*)&Ptrs[j], sizeof(uint32_t));
				FileHandle->Read((uint8_t*)&Sizes[j], sizeof(uint32_t));
			}

			for (uint32_t j = 0; j < ColumnCount; ++j)
			{
				if (!Ptrs[j] || Sizes[j] == 0)
					continue;

				FTableField Entry;

				Entry.Data = MakeShareable(new uint8_t[Sizes[j]]);
				Entry.Size = Sizes[j];

				FileHandle->Read(Entry.Data.Get(), Sizes[j]);

				FTableRowInfo info;
				info.ColumnName = UTF8_TO_TCHAR(ColumnNames[j].c_str());
				info.ColumnID = j;

				row.Fields.Add(info, Entry);
			}

			Result.Add(row);
		}

		delete[] Data;
	}

	delete FileHandle;
	return Result;
}

void TableBinary::Import(const TSharedPtr<FJsonObject>& entries)
{
	m_bListUpdated = true;

	// Get highest ID
	uint32_t highestID = 0;

	for (auto rows = entries->Values.begin(); rows != entries->Values.end(); ++rows)
	{
		highestID = std::max(highestID, (uint32_t)FCString::Atoi(*rows.Key()));
	}

	// Set columns
	auto firstRow = entries->Values.begin().Value()->AsObject();
	m_UDAT.ColumnCount = firstRow->Values.Num();

	// Check change in column
	m_UDAT.CSCT.Entries.SetNum(m_UDAT.ColumnCount);

	uint32_t i = 0;

	for (auto col = firstRow->Values.begin(); col != firstRow->Values.end(); ++col)
	{
		m_UDAT.CSCT.Entries[i].Name = TCHAR_TO_UTF8(*col.Key());
		m_UDAT.CSCT.Entries[i].Type = col.Value()->Type;
		i++;
	}

	// Set data
	m_UDAT.EntityCount = std::max(
		(uint32_t)m_UDAT.EntityCount,
		std::max((uint32_t)entries->Values.Num(), highestID + 1)
	);

	m_UDAT.DSCT.ESCTs.SetNum(m_UDAT.EntityCount);
	m_UDAT.DSCT.Pointers.SetNumZeroed(m_UDAT.EntityCount);

	m_Data.SetNum(m_UDAT.EntityCount * m_UDAT.ColumnCount);

	i = 0;

	for (auto rows = entries->Values.begin(); rows != entries->Values.end(); ++rows)
	{
		auto row = rows->Value->AsObject();
		uint32_t j = FCString::Atoi(*rows.Key()) * m_UDAT.ColumnCount;

		// This entity has an inconsistent amount of column data
		// You may continue, but this need to be fixed asap
		// Stability is not guaranteed
		if (static_cast<uint32>(row->Values.Num()) != m_UDAT.ColumnCount)
		{
			UE_LOG(LogTemp, Error, TEXT("Entity ID %s from table %s has an insufficient amount of column data!"), *rows.Key(), *FString(m_UDAT.Name));
			check(false);
		}

		for (auto entity = row->Values.begin(); entity != row->Values.end(); ++entity)
		{
			m_Data[j].Data.Reset();

			uint32_t l;
			void* d;

			FString strData = entity.Value()->AsString();
			double dData = entity.Value()->AsNumber();
			bool bData = entity.Value()->AsBool();
			std::string sData;

			switch (entity.Value()->Type)
			{
			case EJson::Number:
				l = sizeof(double);
				d = &dData;
				break;
			case EJson::Boolean:
				l = sizeof(uint8_t);
				d = &bData;
				break;
			case EJson::Null:
				l = 0;
				d = nullptr;
				break;
			default:
				sData = std::string(TCHAR_TO_UTF8(*strData));
				l = sData.length();
				d = const_cast<char*>(sData.c_str());
				break;
			}

			m_Data[j].Size = l;

			if (l != 0)
			{
				m_Data[j].Data = MakeShareable(new uint8_t[m_Data[j].Size]);
				memcpy(m_Data[j].Data.Get(), d, m_Data[j].Size);
			}

			j++;
		}

		i++;
	}
}

void TableBinary::Export()
{
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/");
	path.Append(m_UDAT.Name);
	path.AppendChar('.');
	path.Append(FString(&m_Extension[0]));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenWrite(*path);

	// When file cannot be opened
	if (!FileHandle)
	{
		FStringView Str;
		
		// Create if it doesn't exist
		FFileHelper::SaveStringToFile(Str, *path);
		FileHandle = PlatformFile.OpenWrite(*path);

		if (!FileHandle)
		{
			const FText DialogText = FText::Format(
				LOCTEXT("Error while saving table file", "Unable to save table file: \"{0}\"!"),
				FText::FromString(path)
			);

			ADD_NOTIFICATION
			Notification->SetCompletionState(SNotificationItem::CS_Fail);

			return;
		}
	}

	// Fill header data
	// Set timestamp to now
	m_UDAT.Modified = std::time(0);
	m_UDAT.Properties[0] = 0; // Compression flag

	m_UDAT.ColumnCount = m_UDAT.CSCT.Entries.Num();
	m_UDAT.EntityCount = m_Data.Num() / m_UDAT.ColumnCount;

	uint32_t tableDataSize = 0;

	// Data offsets per entity
	TArray<uint32_t> entityDataOffsets;
	entityDataOffsets.SetNum(m_Data.Num());

	m_UDAT.DSCT.Pointers.SetNumZeroed(m_UDAT.EntityCount);

	TArray<uint32_t> entitySizes;
	entitySizes.SetNum(m_UDAT.EntityCount);

	uint32_t entityDataOffset = 0;

	uint32_t e = 0; // number of entries
	uint32_t f = 0; // number of non-deleted entries
	uint32_t g = 0; // number of deleted entries

	TArray<bool> bHasData;
	bHasData.SetNumZeroed(m_UDAT.EntityCount);

	// Determine full data size and calculate offsets for each data in entity
	for (int32_t i = 0; i < m_Data.Num(); ++i)
	{
		// Reset for each entity
		if (i % m_UDAT.ColumnCount == 0)
		{
			bHasData[i / m_UDAT.ColumnCount] = false;

			// Check if data exists
			for (uint32_t j = 0; j < m_UDAT.ColumnCount; ++j)
			{
				if (m_Data[i + j].Size != 0)
				{
					bHasData[i / m_UDAT.ColumnCount] = true;

					// Check if deleted == 1, then exclude
					if (strncmp(m_UDAT.CSCT.Entries[j].Name.c_str(), "deleted", 7) == 0)
					{
						uint8_t dat = 0;
						memcpy(&dat, m_Data[i + j].Data.Get(), sizeof(uint8_t));

						if (dat == 1)
						{
							bHasData[i / m_UDAT.ColumnCount] = false;
							break;
						}
					}
				}
			}

			if (!bHasData[i / m_UDAT.ColumnCount])
			{
				m_UDAT.DSCT.Pointers[e] = 0;
				entitySizes[e] = 0;

				g++;
			}
			else
			{
				m_UDAT.DSCT.Pointers[e] = tableDataSize + 8 * f; // Added ESCT & size == 8 bytes
				entitySizes[e] = 8; // Added ESCT & size == 8 bytes

				f++;
			}

			e++;

			entityDataOffset = 0;
		}

		entityDataOffsets[i] = entityDataOffset;

		if (!bHasData[i / m_UDAT.ColumnCount])
			continue;

		entityDataOffset += m_Data[i].Size;
		entitySizes[e - 1] += m_Data[i].Size + 4 * 2; // Add 4 bytes for additional address data and size
		
		tableDataSize += m_Data[i].Size + 4 * 2; // Add 4 bytes for additional address data and size
	}

	// Determine full column data size
	uint32_t columnDataSize = 8 + m_UDAT.ColumnCount * sizeof(uint32_t); // 8 bytes for magic ID + size, and data pointers

	for (uint32_t i = 0; i < m_UDAT.ColumnCount; ++i)
	{
		columnDataSize += m_UDAT.CSCT.Entries[i].Name.size() + 1; // 1 byte for type
	}

	// Precalculate the full file size
	uint32_t fileSize = m_UDAT.Size; // Header
	fileSize += columnDataSize; // Column data, with magic ID and size
	fileSize += sizeof(uint32_t) * m_UDAT.EntityCount + 4; // DSCT magic ID + Table pointers
	fileSize += sizeof(uint32_t) * m_UDAT.EntityCount * 2; // ESCT magic ID + size
	fileSize += tableDataSize; // Entity pointers and data

	uint8_t* buffer = new uint8_t[fileSize];
	uint32_t offset = 0;

	// Append header
	memcpy(&buffer[offset], &m_UDAT.MagicID, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&buffer[offset], &m_UDAT.Size, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&buffer[offset], &m_UDAT.Version, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&buffer[offset], &m_UDAT.Modified, sizeof(uint64_t));
	offset += sizeof(uint64_t);
	memcpy(&buffer[offset], &m_UDAT.Properties, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&buffer[offset], &m_UDAT.Name, sizeof(char[32]));
	offset += sizeof(char[32]);
	memcpy(&buffer[offset], &m_UDAT.ColumnCount, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&buffer[offset], &m_UDAT.EntityCount, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Append columns, starting new CSCT (Column Section)
	static const char csct[4] = { 'C', 'S', 'C', 'T' };

	memcpy(&buffer[offset], &csct, sizeof(char[4]));
	offset += sizeof(char[4]);

	memcpy(&buffer[offset], &columnDataSize, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	uint32_t columnOffset = offset + m_UDAT.ColumnCount * sizeof(uint32_t);

	// Append all column data pointers
	for (uint32_t i = 0; i < m_UDAT.ColumnCount; ++i)
	{
		memcpy(&buffer[offset], &columnOffset, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		columnOffset += m_UDAT.CSCT.Entries[i].Name.size() + 1; // Added type byte
	}

	// Append all column data
	for (uint32_t i = 0; i < m_UDAT.ColumnCount; ++i)
	{
		memset(&buffer[offset], (int)m_UDAT.CSCT.Entries[i].Type, sizeof(char));
		offset += sizeof(char);

		memcpy(&buffer[offset], m_UDAT.CSCT.Entries[i].Name.c_str(), m_UDAT.CSCT.Entries[i].Name.size());
		offset += m_UDAT.CSCT.Entries[i].Name.size();
	}

	// Append entity address offsets, starting new DSCT (Data Section)
	static const char dsct[4] = { 'D', 'S', 'C', 'T' };

	memcpy(&buffer[offset], &dsct, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	for (uint32_t i = 0; i < m_UDAT.EntityCount; ++i)
	{
		m_UDAT.DSCT.Pointers[i] += offset + sizeof(uint32_t) * m_UDAT.EntityCount;

		if (!bHasData[i])
		{
			m_UDAT.DSCT.Pointers[i] = 0;
		}

		memcpy(&buffer[offset + i * sizeof(uint32_t)], &m_UDAT.DSCT.Pointers[i], sizeof(uint32_t));
	}

	offset += m_UDAT.EntityCount * sizeof(uint32_t);
	e = 0;

	// Append data
	for (int32_t i = 0; i < m_Data.Num(); ++i)
	{
		if (i % m_UDAT.ColumnCount == 0)
		{
			if (!bHasData[i / m_UDAT.ColumnCount])
			{
				i += m_UDAT.ColumnCount - 1;
				e++;

				fileSize -= sizeof(uint32_t) * 2;
				continue;
			}

			// Starting new ESCT (Entity Section)
			static const char esct[4] = { 'E', 'S', 'C' , 'T' };

			memcpy(&buffer[offset], &esct, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			// Append section size
			memcpy(&buffer[offset], &entitySizes[e], sizeof(uint32_t));
			offset += sizeof(uint32_t);

			// Entity data start offset in file
			uint32_t startOffset = offset + sizeof(uint32_t) * 2 * m_UDAT.ColumnCount;

			for (uint32_t j = 0; j < m_UDAT.ColumnCount; ++j)
			{
				if (m_Data[i + j].Size != 0)
					entityDataOffsets[i + j] += startOffset;
				else
					entityDataOffsets[i + j] = 0;

				memcpy(&buffer[offset], &entityDataOffsets[i + j], sizeof(uint32_t));
				offset += sizeof(uint32_t);

				memcpy(&buffer[offset], &m_Data[i + j].Size, sizeof(uint32_t));
				offset += sizeof(uint32_t);
			}

			e++;
		}

		// Insert actual data
		memcpy(&buffer[offset], m_Data[i].Data.Get(), m_Data[i].Size);
		offset += m_Data[i].Size;
	}

	// Write to file
	FileHandle->Write(buffer, fileSize);

	delete FileHandle;
	delete[] buffer;

	const FText DialogText = FText::Format(
		LOCTEXT("Succefully saved table file", "Succesfully saved the table file to:\n\"{0}\"!"),
		FText::FromString(path)
	);

	ADD_NOTIFICATION
	Notification->SetCompletionState(SNotificationItem::CS_Success);
}

const TMap<uint32, FString>& TableBinary::Populate(const FString& columnToShow)
{
	if (m_ShownColumn == columnToShow && !m_bListUpdated)
	{
		return m_List;
	}
	
	m_bListUpdated = false;
	m_ShownColumn = columnToShow;

	uint32_t uiColumnID = GetColumnID(columnToShow);

	for (uint32_t i = 0; i < m_UDAT.EntityCount; ++i)
	{
		if (!m_UDAT.DSCT.Pointers[i])
		{
			m_List.Remove(i);
			continue;
		}

		FString d;

		if (uiColumnID == UINT_MAX)
		{
			d = FString::FromInt(i);
		}
		else
		{
			d += "[" + FString::FromInt(i) + "] ";
			d.AppendChars(
				UTF8_TO_TCHAR(m_Data[i * m_UDAT.ColumnCount + uiColumnID].Data.Get()),
				m_Data[i * m_UDAT.ColumnCount + uiColumnID].Size
			);
		}

		m_List.Add(i, d);
	}

	return m_List;
}

TMap<uint32, FString> TableBinary::PopulateWithLink(const FString& columnToShow, const FString& linkedColumn, uint32 linkedId)
{
	TMap<uint32, FString> NewList = Populate(columnToShow);
	uint32_t uiLinkedID = linkedColumn != "" ? GetColumnID(linkedColumn) : UINT_MAX;

	// Remove if not linked
	if (uiLinkedID != UINT_MAX && linkedColumn != "")
	{
		TArray<uint32> Keys;
		NewList.GetKeys(Keys);

		for (uint32 Key : Keys)
		{
			const FTableField& Field = GetValue(m_UDAT.Name, uiLinkedID, Key);

			double d;
			memcpy(&d, Field.Data.Get(), Field.Size);

			if ((uint32)d != linkedId)
				NewList.Remove(Key);
		}

		NewList.Compact();
	}

	return NewList;
}

uint32_t TableBinary::GetColumnID(const FString& columnName) const
{
	std::string cName = TCHAR_TO_UTF8(*columnName);

	for (uint32_t j = 0; j < m_UDAT.ColumnCount; ++j)
	{
		if (strncmp(m_UDAT.CSCT.Entries[j].Name.c_str(), cName.c_str(), cName.length()) == 0)
			return j;
	}

	return UINT_MAX;
}

FString TableBinary::GetColumnName(uint32 columnId) const
{
	if (columnId > m_UDAT.ColumnCount)
		return "";

	return UTF8_TO_TCHAR(m_UDAT.CSCT.Entries[columnId].Name.c_str());
}

// Retrieve from existing file
void TableBinary::Refresh()
{
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/");
	path.Append(m_UDAT.Name);
	path.AppendChar('.');
	path.Append(FString(&m_Extension[0]));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*path);

	// Do nothing when file cannot be opened
	if (!FileHandle)
	{
		return;
	}

	m_bListUpdated = true;

	// Read and check header
	FileHandle->Read((uint8_t*)&m_UDAT.MagicID, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&m_UDAT.Size, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&m_UDAT.Version, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&m_UDAT.Modified, sizeof(uint64_t));
	FileHandle->Read((uint8_t*)&m_UDAT.Properties, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&m_UDAT.Name, sizeof(char) * 32);
	FileHandle->Read((uint8_t*)&m_UDAT.ColumnCount, sizeof(uint32_t));
	FileHandle->Read((uint8_t*)&m_UDAT.EntityCount, sizeof(uint32_t));

	if (strncmp(m_UDAT.MagicID, "UDAT", 4) != 0)
	{
		const FText DialogText = FText::Format(
			LOCTEXT("Not a valid UDAT file", "The table file is invalid:\n\"{0}\"!"),
			FText::FromString(path)
		);

		ADD_NOTIFICATION
		Notification->SetCompletionState(SNotificationItem::CS_Fail);

		delete FileHandle;
		return;
	}

	/* CSCT */
	uint32_t CSCTPosition = FileHandle->Tell();

	FileHandle->Read((uint8_t*)&m_UDAT.CSCT.MagicID, sizeof(uint32_t));

	if (strncmp(m_UDAT.CSCT.MagicID, "CSCT", 4) != 0)
	{
		const FText DialogText = FText::Format(
			LOCTEXT("Invalid CSCT section", "Detected an invalid CSCT section, the file might possibly be corrupt:\n\"{0}\"!"),
			FText::FromString(path)
		);

		ADD_NOTIFICATION
		Notification->SetCompletionState(SNotificationItem::CS_Fail);

		delete FileHandle;
		return;
	}

	FileHandle->Read((uint8_t*)&m_UDAT.CSCT.Size, sizeof(uint32_t));

	m_UDAT.CSCT.Pointers.SetNumZeroed(m_UDAT.ColumnCount);
	m_UDAT.CSCT.Entries.SetNum(m_UDAT.ColumnCount);

	for (uint32_t i = 0; i < m_UDAT.ColumnCount; ++i)
	{
		FileHandle->Read((uint8_t*)&m_UDAT.CSCT.Pointers[i], sizeof(uint32_t));
	}

	uint32_t dataSize = 0;

	for (uint32_t i = 0; i < m_UDAT.ColumnCount; ++i)
	{
		// Take data chunk size
		if (i == m_UDAT.ColumnCount - 1)
		{
			dataSize = (CSCTPosition + m_UDAT.CSCT.Size) - m_UDAT.CSCT.Pointers[i] - 1;
		}
		// Use next pointer to determine size
		else
		{
			uint32_t p = 0;

			for (uint32_t k = i + 1; k < m_UDAT.ColumnCount; ++k)
			{
				if (m_UDAT.CSCT.Pointers[k])
				{
					p = m_UDAT.CSCT.Pointers[k];
					break;
				}
			}

			dataSize = p - m_UDAT.CSCT.Pointers[i] - 1;
		}

		FileHandle->Read((uint8_t*)&m_UDAT.CSCT.Entries[i].Type, sizeof(uint8_t));

		m_UDAT.CSCT.Entries[i].Name.resize(dataSize + 1);
		FileHandle->Read((uint8_t*)m_UDAT.CSCT.Entries[i].Name.c_str(), dataSize);
	}

	/* DSCT */
	FileHandle->Read((uint8_t*)&m_UDAT.DSCT.MagicID, sizeof(uint32_t));

	if (strncmp(m_UDAT.DSCT.MagicID, "DSCT", 4) != 0)
	{
		const FText DialogText = FText::Format(
			LOCTEXT("Invalid DSCT section", "Detected an invalid DSCT section, the file might possibly be corrupt:\n\"{0}\"!"),
			FText::FromString(path)
		);

		ADD_NOTIFICATION
		Notification->SetCompletionState(SNotificationItem::CS_Fail);

		delete FileHandle;
		return;
	}

	m_UDAT.DSCT.Pointers.SetNumZeroed(m_UDAT.EntityCount);
	m_UDAT.DSCT.ESCTs.SetNum(m_UDAT.EntityCount);

	m_Data.SetNumZeroed(m_UDAT.ColumnCount * m_UDAT.EntityCount);

	// Entity pointers
	for (uint32_t i = 0; i < m_UDAT.EntityCount; ++i)
	{
		FileHandle->Read((uint8_t*)&m_UDAT.DSCT.Pointers[i], sizeof(uint32_t));
	}

	// Entity data
	for (uint32_t i = 0; i < m_UDAT.EntityCount; ++i)
	{
		// Skip null/0 pointers
		if (!m_UDAT.DSCT.Pointers[i])
			continue;

		uint32_t ESCTPosition = FileHandle->Tell();

		/* ESCT */
		FileHandle->Read((uint8_t*)&m_UDAT.DSCT.ESCTs[i].MagicID, sizeof(uint32_t));

		if (strncmp(m_UDAT.DSCT.ESCTs[i].MagicID, "ESCT", 4) != 0)
		{
			const FText DialogText = FText::Format(
				LOCTEXT("Invalid ESCT section", "Detected an invalid ESCT section, the file might possibly be corrupt:\n\"{0}\"!"),
				FText::FromString(path)
			);

			ADD_NOTIFICATION
			Notification->SetCompletionState(SNotificationItem::CS_Fail);

			delete FileHandle;
			return;
		}

		FileHandle->Read((uint8_t*)&m_UDAT.DSCT.ESCTs[i].Size, sizeof(uint32_t));
		m_UDAT.DSCT.ESCTs[i].Pointers.SetNumZeroed(m_UDAT.ColumnCount);
		m_UDAT.DSCT.ESCTs[i].Sizes.SetNumZeroed(m_UDAT.ColumnCount);

		// Read pointers
		for (uint32_t j = 0; j < m_UDAT.ColumnCount; ++j)
		{
			FileHandle->Read((uint8_t*)&m_UDAT.DSCT.ESCTs[i].Pointers[j], sizeof(uint32_t));
			FileHandle->Read((uint8_t*)&m_UDAT.DSCT.ESCTs[i].Sizes[j], sizeof(uint32_t));
		}

		// Read data from pointers
		for (uint32_t j = 0; j < m_UDAT.ColumnCount; ++j)
		{
			// Skip null/0 pointers
			if (!m_UDAT.DSCT.ESCTs[i].Pointers[j] || m_UDAT.DSCT.ESCTs[i].Sizes[j] == 0)
				continue;

			uint32_t id = i * m_UDAT.ColumnCount + j;

			m_Data[id].Data.Reset();

			m_Data[id].Size = m_UDAT.DSCT.ESCTs[i].Sizes[j];
			m_Data[id].Data = MakeShareable(new uint8_t[m_UDAT.DSCT.ESCTs[i].Sizes[j]]);

			FileHandle->Read(m_Data[id].Data.Get(), m_UDAT.DSCT.ESCTs[i].Sizes[j]);
		}
	}

	delete FileHandle;
}

#undef LOCTEXT_NAMESPACE