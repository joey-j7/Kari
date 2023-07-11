#pragma once

#include "CSCT.h"
#include "DSCT.h"

#include <cstdint>

typedef struct UDAT {
	const char MagicID[4] = { 'U', 'D', 'A', 'T' };	// Magic ID; UDAT
	const uint32_t Size = 64;						// Header size (64 bytes)
	const uint32_t Version = 100;					// Format version
	uint64_t Modified = 0;							// Last modified timestamp
	char Properties[4] = { 0, 0, 0, 0 };			// Compression flag, empty data alignment
	char Name[32] = "";								// Table Name
	uint32_t ColumnCount = 0;						// Number of columns within the table
	uint32_t EntityCount = 0;						// Number of entities within the table

	CSCT CSCT;
	DSCT DSCT;
} UDAT;
