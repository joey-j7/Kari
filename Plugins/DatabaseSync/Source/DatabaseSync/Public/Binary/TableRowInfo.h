#pragma once

#include "CoreMinimal.h"
#include "TableRowInfo.Generated.h"

USTRUCT(BlueprintType)
struct FTableRowInfo
{
	GENERATED_BODY()
	FString ColumnName = "";
	uint32 ColumnID = UINT_MAX;

	bool operator==(const FTableRowInfo& a) const
	{
		return ColumnName == a.ColumnName && ColumnID == a.ColumnID;
	}

	bool operator==(uint32 id) const
	{
		return ColumnID == id;
	}

	bool operator !=(uint32 id) const
	{
		return ColumnID != id;
	}

	bool operator==(const FString& s) const
	{
		return ColumnName == s;
	}

	bool operator !=(const FString& s) const
	{
		return ColumnName != s;
	}
};

FORCEINLINE uint32 GetTypeHash(const FTableRowInfo& b)
{
	return FCrc::MemCrc_DEPRECATED(&b, sizeof(FTableRowInfo));
}