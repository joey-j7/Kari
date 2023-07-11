#include "SteamUErManager.h"

#if PLATFORM_WINDOWS
TMap<FString, SteamUErLeaderboards*> SteamUErManager::steamLeaderboards;
#endif