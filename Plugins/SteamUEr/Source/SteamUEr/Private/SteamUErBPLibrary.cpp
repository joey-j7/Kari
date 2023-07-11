// 2017 Turfster
// This basically just interfaces with the Steam API, filling in gaps left by the OnlineSubsystemSteam
// (which you need to have installed if you want to make changes, as per https://docs.unrealengine.com/latest/INT/Programming/Online/Steam/)
// PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

#include "SteamUErBPLibrary.h"

#if PLATFORM_WINDOWS
#pragma push_macro("ARRAY_COUNT")	// duplicate macro definitions in the steam api and the UE api, so we need to do this first
#undef ARRAY_COUNT
#include "ThirdParty/Steamworks/Steamv147/sdk/public/steam/steam_api.h"
#include "ThirdParty/Steamworks/Steamv147/sdk/public/steam/isteamutils.h"
#pragma pop_macro("ARRAY_COUNT")
#include "SteamUErManager.h"

SteamUErGlobalStats* USteamUErBPLibrary::myGlobalStatsObject;
ISteamUserStats* USteamUErBPLibrary::myStats;
ISteamUser* USteamUErBPLibrary::myUser;
bool USteamUErBPLibrary::inShutdownMode;
#endif

USteamUErBPLibrary::USteamUErBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
#if PLATFORM_WINDOWS
	myGlobalStatsObject = new SteamUErGlobalStats();
	inShutdownMode = false;
#endif
}

bool USteamUErBPLibrary::SteamUErIsSteamConnected()
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (SteamUser())
		{
			return SteamUser()->BLoggedOn();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SteamUEr: Problem getting Steam user information."));
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SteamUEr: Steam not running?"));
		return false;
	}
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErInitLeaderboard(FString leaderboardName, int numLeaderboardItems)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::Init({ leaderboardName }, numLeaderboardItems);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErInitLeaderboards(TArray<FString> leaderboardNames, int numLeaderboardItems)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::Init(leaderboardNames, numLeaderboardItems);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErInitOrCreateLeaderboards(TArray<FSteamUErLeaderboardCreationData> leaderboardCreationList)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::Init(leaderboardCreationList);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErCloseLeaderboards()
{
#if PLATFORM_WINDOWS
	return SteamUErManager::Close();
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErPingAsynchronous(bool& scoreSetWaiting, bool& leaderboardGetWaiting, bool& userLeaderboardGetWaiting, bool& globalStatsWaiting, bool& globalPlayerCountWaiting, bool& globalAchievementPercentagesWaiting)
{
#if PLATFORM_WINDOWS
	if (!inShutdownMode)
	{
		globalStatsWaiting = SteamUErGlobalStatsReady();
		globalPlayerCountWaiting = SteamUErCurrentPlayersReady();
		//	globalAchievementPercentagesWaiting = SteamUErGlobalAchievementPercentagesReady();
		return SteamUErManager::Update(scoreSetWaiting, leaderboardGetWaiting, userLeaderboardGetWaiting);
	}
	return false;
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErUploadScoreToLeaderboard(FString leaderboardName, int score, bool useForceUpdate )
{
#if PLATFORM_WINDOWS
	return SteamUErManager::SetScore(leaderboardName, score, useForceUpdate);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErGetLeaderboard(FString leaderboardName, TArray<FSteamUErLeaderboardData>& leaderboardData)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::GetLeaderboardScores(leaderboardName, leaderboardData);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErGetLeaderboardInited(FString leaderboardName)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::GetLeaderboardInited(leaderboardName);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErGetLeaderboardReady(FString leaderboardName)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::GetLeaderboardReady(leaderboardName);
#endif

	return false;
}

TArray<UTexture2D*> USteamUErBPLibrary::SteamUErGetLeaderboardAvatars(FString leaderboardName)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::GetLeaderboardAvatars(leaderboardName);
#endif

	return {};
}

bool USteamUErBPLibrary::SteamUErRequestLeaderboard(FString leaderboardName, int numScores, ESteamUErScoreTypeEnum leaderboardType, int startingIndex)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::RequestLeaderboard(leaderboardName, numScores, leaderboardType, startingIndex);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErRequestLeaderboardFromLeaderboard(FString leaderboardName, FString userNameSourceLeaderboard)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::RequestLeaderboardFromLeaderboard(leaderboardName, userNameSourceLeaderboard);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErRequestPlayerFromLeaderboard(FString leaderboardName)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myUser) myUser = SteamUser();
		if (myUser)
		{
			CSteamID playerID = myUser->GetSteamID();
			return SteamUErManager::RequestPlayerFromLeaderboard(leaderboardName, playerID);
		}
	}
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErUserLeaderboardDataReady(FString leaderboardName)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::GetUserLeaderboardDataReady(leaderboardName);
#endif

	return false;
}

bool USteamUErBPLibrary::SteamUErGetUserLeaderboardData(FString leaderboardName, TArray<FSteamUErLeaderboardData>& leaderboardData)
{
#if PLATFORM_WINDOWS
	return SteamUErManager::GetUserLeaderboardData(leaderboardName, leaderboardData);
#endif

	return false;
}


int USteamUErBPLibrary::SteamUErGetPlayMode(UObject* WorldContextObject)
{
#if PLATFORM_WINDOWS
	if (WorldContextObject)
	{
		if (GEngine != nullptr)
		{
			UWorld* currentWorld = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
			if (currentWorld)
				return (int)currentWorld->WorldType;
		}
	}
#endif
	
	return -1;
}

FString USteamUErBPLibrary::SteamUErConvertDateTimeToString(FDateTime unlockTimestamp) // I could not find a default BP node for this? For some reason?
{
	return unlockTimestamp.ToString();
}


UTexture2D* USteamUErBPLibrary::SteamUErGetPlayerAvatar()
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myUser) myUser = SteamUser();
		if (myUser)
		{
			CSteamID playerID = myUser->GetSteamID();
			return SteamUErManager::SteamUErGetAvatar(playerID);
		}
	}
#endif
	
	return NULL;
}


UTexture2D* USteamUErBPLibrary::SteamUErGetAchievementIcon(FString achievementName)
{
	UTexture2D* result = NULL;

#if PLATFORM_WINDOWS
	if (SteamAPI_Init()) // I don't know if these are strictly *necessary*, but better safe than sorry, right?
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			int iconHandle = myStats->GetAchievementIcon(TCHAR_TO_UTF8(*achievementName));
			if (iconHandle != 0)
			{
				int bufferSize = 0;
				uint32 width = 0;
				uint32 height = 0;
				if (SteamUtils()->GetImageSize(iconHandle, &width, &height))
				{
					uint8* buffer = new uint8[width*height*4];
					if (SteamUtils()->GetImageRGBA(iconHandle, buffer, width*height * 4))
					{
						result = UTexture2D::CreateTransient(width, height, EPixelFormat::PF_R8G8B8A8); // Create a transient texture
						if (result)
						{
							result->CompressionSettings = TextureCompressionSettings::TC_Default;
							result->SRGB = 0;
							result->UpdateResource();
							FTexture2DMipMap &map = result->PlatformData->Mips[0];
							void* data = map.BulkData.Lock(LOCK_READ_WRITE);
							FMemory::Memcpy(data, buffer, width*height * 4); // write the icon data to the texture
							map.BulkData.Unlock();
							result->UpdateResource();
						}
					}
					delete[] buffer;
				}
			}
		}
	}
#endif
	
	return result;
}

bool USteamUErBPLibrary::SteamUErGetAchievementDisplayAttribute(FString achievementName, FString& localisedName, FString& localisedDescription, bool& isHidden)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			FString achievementKey = "name";
			localisedName = UTF8_TO_TCHAR(myStats->GetAchievementDisplayAttribute(TCHAR_TO_UTF8(*achievementName), TCHAR_TO_UTF8(*achievementKey)));
			achievementKey = "desc";
			localisedDescription = UTF8_TO_TCHAR(myStats->GetAchievementDisplayAttribute(TCHAR_TO_UTF8(*achievementName), TCHAR_TO_UTF8(*achievementKey)));
			achievementKey = "hidden";
			isHidden = (*const_cast<char*>(myStats->GetAchievementDisplayAttribute(TCHAR_TO_UTF8(*achievementName), TCHAR_TO_UTF8(*achievementKey))))=='1';
			return true;
		}
	}
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErGetAchievementUnLockedTime(FString achievementName, bool& unlockStatus, FDateTime& unlockTimestamp)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			uint32 unlockTime = 1;
			myStats->GetAchievementAndUnlockTime(TCHAR_TO_UTF8(*achievementName), &unlockStatus, &unlockTime);
			unlockTimestamp = FDateTime::FromUnixTimestamp(unlockTime);
			return true;
		}
		else
			return false;
	}
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErSetAchievement(FString achievementName)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats != NULL)
		{
			if (myStats->SetAchievement(TCHAR_TO_UTF8(*achievementName)))
			{
				myStats->StoreStats();
				return true;
			}
			else
				UE_LOG(LogTemp, Error, TEXT("Could not set achievement %s"), *achievementName);
		}
		else
			UE_LOG(LogTemp, Error, TEXT("No stats?"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Steam not initialized?"));
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErGetAchievement(FString achievementName)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats != NULL)
		{
			bool achieved = false;
			if (myStats->GetAchievement(TCHAR_TO_UTF8(*achievementName),&achieved))
			{
				return achieved;
			}
		}
	}
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErClearAchievement(FString achievementName)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats != NULL)
		{
			if (myStats->ClearAchievement(TCHAR_TO_UTF8(*achievementName)))
			{
				myStats->StoreStats();
				return true;
			}
		}
	}
#endif
	
	return false;
}

int USteamUErBPLibrary::SteamUErNumAchievements()
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats != NULL)
		{
			return (myStats->GetNumAchievements());
		}
	}
#endif
	
	return -1;
}

FString USteamUErBPLibrary::SteamUErGetAchievementName(int32 achievementNumber)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats != NULL)
		{
			return UTF8_TO_TCHAR(myStats->GetAchievementName(achievementNumber));
		}
	}
#endif
	
	return "UNKNOWN";
}

bool USteamUErBPLibrary::SteamUErResetAllStats(bool achievementsToo)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats != NULL)
		{
			return (myStats->ResetAllStats(achievementsToo));
		}
	}
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErSetStatInt(FString statName, int32 value)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats!=NULL)
		{
			if (myStats->SetStat(TCHAR_TO_UTF8(*statName), value))
			{
				myStats->StoreStats();
				return true;
			}
			else
				UE_LOG(LogTemp, Error, TEXT("Could not set stat %s"), *statName);
		}
		else
			UE_LOG(LogTemp, Error, TEXT("No stats?"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Steam not initialized?"));
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErSetStatFloat(FString statName, float value)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			if (myStats->SetStat(TCHAR_TO_UTF8(*statName), value))
			{
				myStats->StoreStats();
				return true;
			}
			else
				UE_LOG(LogTemp, Log, TEXT("Failed to set stat %s!"), *statName);
		}
	}
#endif
	
	return false;
}

int32 USteamUErBPLibrary::SteamUErGetStatInt(FString statName)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			int32 value = 0;
			if (myStats->GetStat(TCHAR_TO_UTF8(*statName), &value))
			{
				return value;
			}
			else
				UE_LOG(LogTemp, Log, TEXT("Failed to get stat %s!"),*statName);
		}
		else
			return -1;
	}
#endif
	
	return -1;
}

bool USteamUErBPLibrary::SteamUErGetStatIntBool(FString statName, int32& value)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
			return myStats->GetStat(TCHAR_TO_UTF8(*statName), &value);
	}
#endif
	
	return false;
}

float USteamUErBPLibrary::SteamUErGetStatFloat(FString statName)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			float value = 0.f;
			myStats->GetStat(TCHAR_TO_UTF8(*statName), &value);
			return value;
		}
		else
			UE_LOG(LogTemp, Log, TEXT("Problem getting stat %s!"), *statName);
	}
#endif
	
	return -1.f;
}

bool USteamUErBPLibrary::SteamUErGetStatFloatBool(FString statName, float& value)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
			return myStats->GetStat(TCHAR_TO_UTF8(*statName), &value);
	}
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErRequestGlobalStats(int numDays)
{
#if PLATFORM_WINDOWS
	if (myGlobalStatsObject)
	{
		myGlobalStatsObject->RequestGlobalStats(numDays);
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("SteamUEr: No stats object?!"));
#endif
	
	return false;
}
	

bool USteamUErBPLibrary::SteamUErRequestCurrentPlayers()
{
#if PLATFORM_WINDOWS
	if (myGlobalStatsObject)
	{
		myGlobalStatsObject->RequestCurrentPlayers();
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("SteamUEr: No stats object?!"));
#endif
	
	return false;
}
/*
bool USteamUErBPLibrary::SteamUErRequestAchievementPercentages()
{
	if (!myGlobalStatsObject)
		myGlobalStatsObject = new SteamUErGlobalStats();

	myGlobalStatsObject->RequestGlobalAchievementPercentages();
	return true;
}*/

bool USteamUErBPLibrary::SteamUErCurrentPlayersReady()
{
#if PLATFORM_WINDOWS
	if (myGlobalStatsObject)
	{
		return myGlobalStatsObject->currentPlayersReady;
	}
#endif
	
	return false;
}

bool USteamUErBPLibrary::SteamUErGlobalStatsReady()
{
#if PLATFORM_WINDOWS
	if (!myGlobalStatsObject)
	{
		UE_LOG(LogTemp, Error, TEXT("SteamUEr: Global stats object not initialized before global stats ready called!"));
		myGlobalStatsObject = new SteamUErGlobalStats();
		// obviously not
		return false;
	}

	return myGlobalStatsObject->globalStatsReady;
#endif

	return false;
}

/*
bool USteamUErBPLibrary::SteamUErGlobalAchievementPercentagesReady()
{
	if (myGlobalStatsObject)
	{
		return myGlobalStatsObject->globalAchievementPercentagesReady;
	}
	return false;
}*/

int USteamUErBPLibrary::SteamUErCurrentPlayers()
{
#if PLATFORM_WINDOWS
	if (myGlobalStatsObject)
	{
		if (myGlobalStatsObject->currentPlayersReady)
			return myGlobalStatsObject->currentPlayers;
	}
#endif
	
	return -1;
}

void USteamUErBPLibrary::SteamUErInvalidateGlobalStats()
{
#if PLATFORM_WINDOWS
	if (myGlobalStatsObject)
	{
		myGlobalStatsObject->globalStatsReady = false;
	}
#endif
}

bool USteamUErBPLibrary::SteamUErGetGlobalStatIntBool(FString statName, FString& value)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			int64 integerValue = 0;
			bool success = myStats->GetGlobalStat(TCHAR_TO_UTF8(*statName), &integerValue);
			char buffer[20];
			sprintf(buffer, "%lli", integerValue);
			value = FString(UTF8_TO_TCHAR(buffer));
			return success;
		}
	}
#endif
	
	return false;

}

bool USteamUErBPLibrary::SteamUErGetGlobalStatFloatBool(FString statName, float& value)
{
#if PLATFORM_WINDOWS
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
			return myStats->GetStat(TCHAR_TO_UTF8(*statName), &value);
	}
#endif
	
	return false;
}