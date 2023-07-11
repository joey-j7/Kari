#pragma once
#include "LoadReasons.generated.h"

UENUM(BlueprintType)
enum class ELevelLoadReason : uint8
{
	/** No level loading has occured yet */
	None,
	/** The level was loaded to move Kari between levels */
	Traversal,
	/** The level was loaded to place Kari in the last saved level */
	Loading,
	/** The level was loaded to respawn Kari in the last saved level */
	Respawning,
	/** The level was loaded after starting a new game */
	NewGame
};
