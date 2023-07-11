// this is a dirty, dirty hack, but... given that the whole thing needs to work with a blueprint library, I don't really have much choice, do I?
// Moved out here to have a clean place and not have everything jumbled together in the damn blueprint library cpp/h files

#pragma once

#if PLATFORM_WINDOWS
#include "SteamUErLeaderboards.h"
#include "SteamUErLeaderboardData.h"
#include "SteamUErLeaderboardCreationData.h"
#include "SteamUErGlobalStats.h"

class SteamUErManager
{

public:

	static TMap<FString, SteamUErLeaderboards*> steamLeaderboards;

	static bool Init(TArray<FString> leaderboardNames, int numLeaderboardItems)
	{
		if (SteamAPI_Init())
		{
			if (steamLeaderboards.Num()>0)
			{
				UE_LOG(LogTemp, Warning, TEXT("SteamUEr: Leaderboards were already opened. This should only be called *once* near program start, not every time a level gets loaded or a player dies."));
				Close();
			}
			UE_LOG(LogTemp, Log, TEXT("SteamUEr: Opened leaderboard manager"));

			for (int i = 0; i < leaderboardNames.Num(); i++)
			{
				steamLeaderboards.Add(leaderboardNames[i], new SteamUErLeaderboards(leaderboardNames[i], numLeaderboardItems));
				steamLeaderboards[leaderboardNames[i]]->FindLeaderboard();
			}

			return true;
		}
		else
			UE_LOG(LogTemp, Error, TEXT("SteamUEr: Steam not initialized?"));
		return false;
	}

	static bool Init(TArray<FSteamUErLeaderboardCreationData> leaderboardCreationList)
	{
		if (SteamAPI_Init())
		{
			if (steamLeaderboards.Num()>0)
			{ 
				UE_LOG(LogTemp, Warning, TEXT("SteamUEr: Leaderboards were already opened. This should only be called *once* near program start, not every time a level gets loaded or a player dies."));
				Close();
			}
			UE_LOG(LogTemp, Log, TEXT("SteamUEr: Opened leaderboard manager"));

			for (int i = 0; i < leaderboardCreationList.Num(); i++)
			{
				steamLeaderboards.Add(leaderboardCreationList[i].leaderboardName, new SteamUErLeaderboards(leaderboardCreationList[i].leaderboardName, leaderboardCreationList[i].leaderboardItems));
				steamLeaderboards[leaderboardCreationList[i].leaderboardName]->FindOrCreateLeaderboard(leaderboardCreationList[i]);
			}

			return true;
		}
		else
			UE_LOG(LogTemp, Error, TEXT("SteamUEr: Steam not initialized?"));
		return false;
	}

	static bool SetScore(FString leaderboardName, int score, bool useForceUpdate)
	{
		if (SteamAPI_Init())
		{
			if (steamLeaderboards.Contains(leaderboardName))
				if (steamLeaderboards[leaderboardName])
					return steamLeaderboards[leaderboardName]->UploadScore(score, useForceUpdate);
		}
		return false;
	}

	static TArray<CSteamID> GetLeaderboardUserIDs(FString leaderboardName)
	{
		TArray<CSteamID> result;
		if (SteamAPI_Init())
			if (steamLeaderboards.Contains(leaderboardName))	// are we tracking the leaderboard
				if (steamLeaderboards[leaderboardName])			// does it exist
					if (steamLeaderboards[leaderboardName]->bLeaderboardScoresFound || steamLeaderboards[leaderboardName]->bLeaderboardScoresCached)	// did we actually get scores
					{
						for (int i = 0; i < steamLeaderboards[leaderboardName]->m_nLeaderboardEntries; i++)
							result.Add(steamLeaderboards[leaderboardName]->m_leaderboardEntries[i].m_steamIDUser);
					}
		return result;
	}

	static bool RequestLeaderboardFromLeaderboard(FString leaderboardName, FString userNamesourceLeaderboard)
	{
		if (SteamAPI_Init())
		{
			TArray<CSteamID> listToGet;
			listToGet = GetLeaderboardUserIDs(userNamesourceLeaderboard);
			if (listToGet.Num() > 0)
			{
				if (steamLeaderboards.Contains(leaderboardName))
					if (steamLeaderboards[leaderboardName])
					{
						steamLeaderboards[leaderboardName]->DownloadScoresForUsers(listToGet);
						return true;
					}
			}
		}

		return false;
	}

	static bool RequestPlayerFromLeaderboard(FString leaderboardName, CSteamID playerID)
	{
		if (SteamAPI_Init())
		{
			TArray<CSteamID> listToGet = {playerID};
			if (listToGet.Num() > 0)
			{
				if (steamLeaderboards.Contains(leaderboardName))
					if (steamLeaderboards[leaderboardName])
					{
						steamLeaderboards[leaderboardName]->DownloadScoresForUsers(listToGet);
						return true;
					}
			}
		}

		return false;
	}

	static bool GetUserLeaderboardDataReady(FString leaderboardName)
	{
		if (SteamAPI_Init())
		{
			if (steamLeaderboards.Contains(leaderboardName))
				if (steamLeaderboards[leaderboardName])
					return steamLeaderboards[leaderboardName]->bLeaderboardUsersFound;
		}
		return false;
	}

	static bool GetUserLeaderboardData(FString leaderboardName, TArray<FSteamUErLeaderboardData>& leaderboardData)
	{
		if (SteamAPI_Init())
			if (steamLeaderboards.Contains(leaderboardName))	// are we tracking the leaderboard
				if (steamLeaderboards[leaderboardName])			// does it exist
				{
					if (steamLeaderboards[leaderboardName]->bLeaderboardUsersFound)
					{
						leaderboardData = steamLeaderboards[leaderboardName]->requestedLeaderboardUserData;
						steamLeaderboards[leaderboardName]->bLeaderboardUsersFound = false;
						return true;
					}
				}
		return false;
	}

	static bool GetLeaderboardScores(FString leaderboardName, TArray<FSteamUErLeaderboardData>& leaderboardData)
	{
		if (SteamAPI_Init())
			if (steamLeaderboards.Contains(leaderboardName))	// are we tracking the leaderboard
				if (steamLeaderboards[leaderboardName])			// does it exist
				{
					if (steamLeaderboards[leaderboardName]->bLeaderboardScoresFound || steamLeaderboards[leaderboardName]->bLeaderboardScoresCached)	// did we actually get scores
					{
						leaderboardData.Empty();
						for (int i = 0; i < steamLeaderboards[leaderboardName]->m_nLeaderboardEntries; i++)
						{
							leaderboardData.Add(FSteamUErLeaderboardData(UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(steamLeaderboards[leaderboardName]->m_leaderboardEntries[i].m_steamIDUser)),
								steamLeaderboards[leaderboardName]->m_leaderboardEntries[i].m_nGlobalRank,
								steamLeaderboards[leaderboardName]->m_leaderboardEntries[i].m_nScore)
								);
						}
						steamLeaderboards[leaderboardName]->bLeaderboardScoresFound = false;
						return true;
					}
				}
		return false;
	}

	static bool GetLeaderboardInited(FString leaderboardName)
	{
		if (SteamAPI_Init())
		{
			if (steamLeaderboards.Contains(leaderboardName))
			{
				if (steamLeaderboards[leaderboardName])
					return (steamLeaderboards[leaderboardName]->LeaderboardInited());
			}
		}
		return false;
	}

	static bool GetLeaderboardReady(FString leaderboardName)
	{
		if (SteamAPI_Init())
		{
			if (steamLeaderboards.Contains(leaderboardName))
				if (steamLeaderboards[leaderboardName])
					return steamLeaderboards[leaderboardName]->bLeaderboardScoresFound;
		}
		return false;
	}

	static bool RequestLeaderboard(FString leaderboardName, int numScores, ESteamUErScoreTypeEnum leaderboardType, int startingIndex = 0)
	{
		if (SteamAPI_Init())
		{
			if (steamLeaderboards.Contains(leaderboardName))
				if (steamLeaderboards[leaderboardName])
					return steamLeaderboards[leaderboardName]->DownloadScores(numScores, leaderboardType, startingIndex);
		}
		return false;
	}

	static bool Update(bool& scoreSetWaiting, bool& leaderboardDataWaiting, bool& userLeaderboardDataWaiting)
	{
		scoreSetWaiting = false;
		leaderboardDataWaiting = false;
		if (SteamAPI_Init())
		{
			SteamAPI_RunCallbacks();
			if (steamLeaderboards.Num()>0)
			{
				for (auto Leaderboard : steamLeaderboards)
				{
					SteamUErLeaderboards* workLB = Leaderboard.Value;
					if (workLB)
					{
						if (workLB->bLeaderboardScoreWaiting)
						{
							scoreSetWaiting = !workLB->UploadWaitingScore();
						}
						else
						{
							if (workLB->bLeaderboardDownloadRequested)
								workLB->DownloadScoresWaiting();

							leaderboardDataWaiting |= workLB->bLeaderboardScoresFound;
							userLeaderboardDataWaiting |= workLB->bLeaderboardUsersFound;
						}

					}
				}
			}
		}
		return false;
	}

	static bool Close() // static class destructor doesn't seem to be called (or is called after the log file is closed. Better be safe, tho)
	{
		UE_LOG(LogTemp, Log, TEXT("SteamUEr: Closing down leaderboard manager"));
		if (steamLeaderboards.Num() > 0)
		{
			TArray<FString> keys;
			int num = steamLeaderboards.GetKeys(keys);
			for (int i = 0; i < num; i++)
				delete steamLeaderboards[keys[i]];
		}
		steamLeaderboards.Empty();
		return true;
	}
	
	static TArray<UTexture2D*> GetLeaderboardAvatars(FString leaderboardName)
	{
		TArray<UTexture2D*> results;
		if (SteamAPI_Init())
			if (steamLeaderboards.Contains(leaderboardName))	// are we tracking the leaderboard
				if (steamLeaderboards[leaderboardName])			// does it exist
				{
					if (steamLeaderboards[leaderboardName]->bLeaderboardScoresCached)	// did we actually get scores and are they still cached?
					{
						for (int i = 0; i < steamLeaderboards[leaderboardName]->m_nLeaderboardEntries; i++)
							results.Add(SteamUErGetAvatar(steamLeaderboards[leaderboardName]->m_leaderboardEntries[i].m_steamIDUser));
					}
				}
		return results;

	}

	static UTexture2D* SteamUErGetAvatar(CSteamID playerID) // moved this here just in case we want to add functionality to get leaderboard player icons
	{
		UTexture2D* result = NULL;

		if (SteamAPI_Init()) // I don't know if these are strictly *necessary*, but better safe than sorry, right?
		{
			ISteamFriends* myFriends = SteamFriends();
			if (myFriends)
			{
				int iconHandle = myFriends->GetLargeFriendAvatar(playerID);
				if (iconHandle != 0)
				{
					int bufferSize = 0;
					uint32 width = 0;
					uint32 height = 0;
					if (SteamUtils()->GetImageSize(iconHandle, &width, &height))
					{
						uint8* buffer = new uint8[width*height * 4];
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
		return result;
	}


};
#endif