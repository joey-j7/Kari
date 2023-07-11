#pragma once

#include "CoreMinimal.h"

#include "TableRowInfo.h"
#include "TableField.h"

#include "TableRow.generated.h"

USTRUCT(BlueprintType)
struct FTableRow
{
	GENERATED_BODY()
	TMap<FTableRowInfo, FTableField> Fields;
};