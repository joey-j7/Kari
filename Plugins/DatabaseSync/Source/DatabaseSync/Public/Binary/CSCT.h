#pragma once

#include <string>
#include <stdint.h>

#include "Core.h"
#include "Json.h"

typedef struct CSCTEntry {
	EJson Type = EJson::None;	// Column type
	std::string Name = "";		// Column name
} CSCTEntry;

typedef struct CSCT {
	const char MagicID[4] = { 'C', 'S', 'C', 'T' };
	uint32_t Size = 0;			// CSCT data size
	TArray<uint32_t> Pointers;	// Column pointers
	TArray<CSCTEntry> Entries;	// Column entries
} CSCT;