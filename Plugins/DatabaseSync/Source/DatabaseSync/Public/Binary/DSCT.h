#pragma once

#include "ESCT.h"

typedef struct DSCT {
	const char MagicID[4] = { 'D', 'S', 'C', 'T' };
	TArray<uint32_t> Pointers;	// Column pointers
	TArray<ESCT> ESCTs;		// Column entries
} DSCT;