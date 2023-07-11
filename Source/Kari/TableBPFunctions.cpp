#include "TableBPFunctions.h"
#include "TableObject.h"

#include <string>

FString UTableBPFunctions::GetTableString(const FString& TableName, const FString& FieldName, int32 ID)
{
	UTableObject* TableObject = NewObject<UTableObject>();
	const FTableField* Field = TableObject->GetField(TableName, FieldName, ID);

	if (!Field || Field->Size == 0)
		return "";

	std::string s;
	s.resize(Field->Size);
	memcpy((void*)s.c_str(), Field->Data.Get(), Field->Size);

	return s.c_str();
}

float UTableBPFunctions::GetTableFloat(const FString& TableName, const FString& FieldName, int32 ID)
{
	UTableObject* TableObject = NewObject<UTableObject>();
	const FTableField* Field = TableObject->GetField(TableName, FieldName, ID);

	if (!Field || Field->Size == 0)
		return 0.f;

	float f = 0.f;
	memcpy((void*)&f, Field->Data.Get(), FMath::Min((uint32)sizeof(float), Field->Size));

	return f;
}

int32 UTableBPFunctions::GetTableInt(const FString& TableName, const FString& FieldName, int32 ID)
{
	UTableObject* TableObject = NewObject<UTableObject>();
	const FTableField* Field = TableObject->GetField(TableName, FieldName, ID);

	if (!Field || Field->Size == 0)
		return 0;

	int32 i = 0;
	memcpy((void*)&i, Field->Data.Get(), FMath::Min((uint32)sizeof(int32), Field->Size));

	return i;
}