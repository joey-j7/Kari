#include "SteamUErGlobalStats.h"

#if PLATFORM_WINDOWS
void SteamUErGlobalStats::RequestGlobalStats(int days)
{
	globalStatsReady = false;
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			UE_LOG(LogTemp, Log, TEXT("SteamUEr: Requesting global stats"));
			SteamAPICall_t hSteamAPICall = myStats->RequestGlobalStats(days);
			m_callGlobalStatsReceived.Set(hSteamAPICall, this, &SteamUErGlobalStats::OnRequestedGlobalStats); // we can't assign a static class to this, so... here we are
		}
	}
	else
		UE_LOG(LogTemp, Error, TEXT("No Steam API?"));
}


void SteamUErGlobalStats::OnRequestedGlobalStats(GlobalStatsReceived_t *pCallback, bool bIOFailure)
{
	if (!bIOFailure)
	{
		globalStatsReady = (pCallback->m_eResult == EResult::k_EResultOK);
	}
}

void SteamUErGlobalStats::RequestCurrentPlayers()
{
	currentPlayersReady = false;
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			UE_LOG(LogTemp, Log, TEXT("SteamUEr: Requesting current player count"));
			SteamAPICall_t hSteamAPICall = myStats->GetNumberOfCurrentPlayers();
			m_callCurrentPlayersReceived.Set(hSteamAPICall, this, &SteamUErGlobalStats::OnRequestedCurrentPlayers); // we can't assign a static class to this, so... here we are
		}
	}
}


void SteamUErGlobalStats::OnRequestedCurrentPlayers(NumberOfCurrentPlayers_t* pCallback, bool bIOFailure)
{
	if (!bIOFailure)
	{
		currentPlayersReady = (pCallback->m_bSuccess == 1);
		if (currentPlayersReady)
		{
			currentPlayers = pCallback->m_cPlayers;
			UE_LOG(LogTemp, Log, TEXT("Current number of players: %i"), currentPlayers);
		}
	}
}



void SteamUErGlobalStats::RequestGlobalAchievementPercentages()
{
	globalAchievementPercentagesReady = false;
	if (SteamAPI_Init())
	{
		if (!myStats) myStats = SteamUserStats();
		if (myStats)
		{
			UE_LOG(LogTemp, Log, TEXT("SteamUEr: Requesting global achievement percentages"));
			SteamAPICall_t hSteamAPICall = myStats->RequestGlobalAchievementPercentages();
			m_callGlobalAchievementPercentagesReceived.Set(hSteamAPICall, this, &SteamUErGlobalStats::OnRequestedGlobalAchievementPercentages);
		}
	}
}


void SteamUErGlobalStats::OnRequestedGlobalAchievementPercentages(GlobalAchievementPercentagesReady_t* pCallback, bool bIOFailure)
{
	if (!bIOFailure)
	{
		globalAchievementPercentagesReady = (pCallback->m_eResult == EResult::k_EResultOK);
		if (globalAchievementPercentagesReady)
		{
			globalAchievementPercentages.Empty();
			if (!myStats) myStats = SteamUserStats();
			if (myStats)
			{
				int bufLen = 17;
				char* achievementName = new char[bufLen]{'A','C','H','_','B','O','T','T','L','E','_','K','I','L','L','S'};
				float percentage = 0;
				bool achieved = false;
				int index = myStats->GetMostAchievedAchievementInfo(achievementName, bufLen, &percentage, &achieved);
				if (index > -1)
				{
					UE_LOG(LogTemp, Log, TEXT("SteamUEr: Index %i"), index);
					FString achievementName2 = FString(UTF8_TO_TCHAR(achievementName));
					UE_LOG(LogTemp, Log, TEXT("SteamUEr: chievo %s"), *achievementName2);
				}
				else
					UE_LOG(LogTemp, Log, TEXT("SteamUEr: No index!"));
				delete[] achievementName;
			}
		}
		else
			UE_LOG(LogTemp, Log, TEXT("SteamUEr: Not Ready!"));
	}
}
#endif