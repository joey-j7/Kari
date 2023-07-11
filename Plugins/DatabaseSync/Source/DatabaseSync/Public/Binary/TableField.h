#pragma once

#include <stdint.h>
#include "TableField.Generated.h"

USTRUCT()
struct FTableField
{
	GENERATED_BODY()

	FTableField() = default;
	FTableField(const FTableField& a)
	{
		*this = a;
	}

	FTableField& operator=(const FTableField& a)
	{
		Size = a.Size;
		Data = a.Data;

		return *this;
	}

	uint32_t Size = 0;
	TSharedPtr<uint8_t> Data;
};