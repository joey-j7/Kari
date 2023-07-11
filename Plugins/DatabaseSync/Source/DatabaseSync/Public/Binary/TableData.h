#pragma once

#include "CoreMinimal.h"

#include "TableRow.h"

#include "TableData.Generated.h"

USTRUCT(BlueprintType)
struct FTableData
{
	GENERATED_BODY()
	TMap<uint32, FTableRow> Rows = TMap<uint32, FTableRow>();
};