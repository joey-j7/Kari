#include "TableWidget.h"
#include "TableDatabase.h"

const FString UTableWidget::GetField(const FString& tableName, const FString& columnName, int32 id) const
{
	std::string s;
	auto data = TableDatabase::Get().GetField(tableName, columnName, id);

	if (data)
	{
		s.resize(data->Size);
		memcpy((void*)s.c_str(), (void*)data->Data.Get(), data->Size);
	}

	return UTF8_TO_TCHAR(s.c_str());
}

TArray<FString> UTableWidget::GetRows(const FString& tableName, const FString& columnName) const
{
	auto& rows = TableDatabase::Get().GetRows(tableName);
	TArray<FString> result;

	const uint32 columnId = TableDatabase::Get().GetBinary(tableName).GetColumnID(columnName);

	for (auto& row : rows)
	{
		for (auto& field : row.Value.Fields)
		{
			if (field.Key != columnId)
				continue;

			std::string s;

			if (field.Value.Data && field.Value.Size > 0)
			{
				s.resize(field.Value.Size);
				memcpy((void*)s.c_str(), (void*)field.Value.Data.Get(), field.Value.Size);
			}

			result.Add(UTF8_TO_TCHAR(s.c_str()));
		}
	}

	return result;
}
