#pragma once

#include <stdint.h>
#include "Core.h"

typedef struct ESCT {
	const char MagicID[4] = { 'E', 'S', 'C', 'T' };	// Magic ID
	uint32_t Size = 0;								// Size
	TArray<uint32_t> Pointers;						// Entity pointers
	TArray<uint32_t> Sizes;							// Entity sizes
	// Entity data
} ESCT;