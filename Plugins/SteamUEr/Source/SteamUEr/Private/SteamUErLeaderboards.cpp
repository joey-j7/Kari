// most of this comes from an edited Spacewar example
#include "SteamUErLeaderboards.h"

#if PLATFORM_WINDOWS
SteamUErLeaderboards::SteamUErLeaderboards(FString leaderboardName, int inLeaderboardEntries) :
	m_nLeaderboardEntries{0},
	m_CurrentLeaderboard{0}
{
	myLeaderboardName = new char[leaderboardName.Len() + 1];
	strcpy(myLeaderboardName, TCHAR_TO_UTF8(*leaderboardName));

	k_nMaxLeaderboardEntries = inLeaderboardEntries;
	m_leaderboardEntries = new LeaderboardEntry_t[k_nMaxLeaderboardEntries];
}

void SteamUErLeaderboards::FindLeaderboard()
{
	m_CurrentLeaderboard = 0;
	SteamAPICall_t hSteamAPICall = SteamUserStats()->FindLeaderboard(myLeaderboardName);
	m_callResultFindLeaderboard.Set(hSteamAPICall, this,
		&SteamUErLeaderboards::OnFindLeaderboard);
	bLeaderboardInitRequested = true;
}

void SteamUErLeaderboards::FindOrCreateLeaderboard(FSteamUErLeaderboardCreationData& inputData)
{
	m_CurrentLeaderboard = 0;

	SteamAPICall_t hSteamAPICall = SteamUserStats()->FindOrCreateLeaderboard(myLeaderboardName, (ELeaderboardSortMethod)inputData.leaderboardSortMethod, (ELeaderboardDisplayType)inputData.leaderboardType);
	m_callResultFindLeaderboard.Set(hSteamAPICall, this,
		&SteamUErLeaderboards::OnFindLeaderboard);
	bLeaderboardInitRequested = true;
}

void SteamUErLeaderboards::OnFindLeaderboard(LeaderboardFindResult_t *pCallback, bool bIOFailure)
{
	// see if we encountered an error during the call
	if (bIOFailure)
	{
		UE_LOG(LogTemp, Error, TEXT("SteamUEr: Steam IO Failure!"));
		return;
	}
	if (!pCallback->m_bLeaderboardFound)
	{
		UE_LOG(LogTemp, Error, TEXT("SteamUEr: Leaderboard could not be found!"));
		return;
	}

	m_CurrentLeaderboard = pCallback->m_hSteamLeaderboard;
	if (m_CurrentLeaderboard != 0){
		bLeaderboardFound = true;
	}
}

bool SteamUErLeaderboards::LeaderboardInited()
{
	if (bLeaderboardInitRequested)
	{
		if (bLeaderboardFound)
			if (m_CurrentLeaderboard != 0) // make sure we actually have something to call
				return true;
	}
/*
	else
		FindLeaderboard(); // we didn't even try to inited yet... // should probably not have this tho
		*/
	return false;
}


bool SteamUErLeaderboards::UploadWaitingScore()
{
	 // Retrying waiting UploadScore 

	if (!m_CurrentLeaderboard) {
		// No leaderboard
		return false;
	}

	SteamAPICall_t hSteamAPICall =
		SteamUserStats()->UploadLeaderboardScore(m_CurrentLeaderboard, waitingScoreUploadForce ? k_ELeaderboardUploadScoreMethodForceUpdate : k_ELeaderboardUploadScoreMethodKeepBest, waitingScore, NULL, 0);
	m_callResultUploadScore.Set(hSteamAPICall, this, &SteamUErLeaderboards::OnUploadScore);

	bLeaderboardScoreWaiting = false;
	return true;
}

bool SteamUErLeaderboards::UploadScore(int score, bool switchToForceUpdate)
{
	bLeaderboardScoreWaiting = true;
	waitingScore = score;
	waitingScoreUploadForce = switchToForceUpdate;
	

	if (!m_CurrentLeaderboard) {
		// No leaderboard
		return false;
	}

	SteamAPICall_t hSteamAPICall =
		SteamUserStats()->UploadLeaderboardScore(m_CurrentLeaderboard, waitingScoreUploadForce ? k_ELeaderboardUploadScoreMethodForceUpdate : k_ELeaderboardUploadScoreMethodKeepBest, score, NULL, 0);
	m_callResultUploadScore.Set(hSteamAPICall, this, &SteamUErLeaderboards::OnUploadScore);

	bLeaderboardScoreWaiting = false;
	return true;
}

void SteamUErLeaderboards::OnUploadScore(LeaderboardScoreUploaded_t *pCallback, bool bIOFailure)
{
	if (!pCallback->m_bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("SteamUEr: score could not be uploaded to Steam"));
	}
	/*if (pCallback->m_bScoreChanged)
	{
		// could display new rank
	}*/

	if (bIOFailure)
		UE_LOG(LogTemp, Error, TEXT("SteamUEr: Steam IO Failure!"));
}

bool SteamUErLeaderboards::DownloadScoresWaiting()
{
	return DownloadScores(waitingDownloadNum, cachedLeaderboardType, waitingLeaderboardStart);
}

bool SteamUErLeaderboards::DownloadScores(int NumberOfScores, ESteamUErScoreTypeEnum leaderboardType, int startingIndex)
{
	cachedLeaderboardType = leaderboardType;
	waitingDownloadNum = NumberOfScores;
	waitingLeaderboardStart = startingIndex;
	bLeaderboardDownloadRequested = true;
	bLeaderboardScoresCached = false;
	bLeaderboardScoresFound = false;
	

	if (!m_CurrentLeaderboard)
		return false;

	SteamAPICall_t hSteamAPICall = 0;

	switch (leaderboardType)
	{
	case ESteamUErScoreTypeEnum::Global: 	hSteamAPICall = SteamUserStats()->DownloadLeaderboardEntries(
		m_CurrentLeaderboard, k_ELeaderboardDataRequestGlobal, startingIndex, startingIndex + FMath::Min(NumberOfScores, k_nMaxLeaderboardEntries));
		break;
	case ESteamUErScoreTypeEnum::AroundUser:	hSteamAPICall = SteamUserStats()->DownloadLeaderboardEntries(
		m_CurrentLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, -NumberOfScores / 2, -NumberOfScores / 2 + FMath::Min(NumberOfScores, k_nMaxLeaderboardEntries));
		break;
	case ESteamUErScoreTypeEnum::Friends:	hSteamAPICall = SteamUserStats()->DownloadLeaderboardEntries(
		m_CurrentLeaderboard, k_ELeaderboardDataRequestFriends, startingIndex, startingIndex + FMath::Min(NumberOfScores, k_nMaxLeaderboardEntries));
		break;
	}
	if (!hSteamAPICall)
		return false;

	m_callResultDownloadScores.Set(hSteamAPICall, this,
		&SteamUErLeaderboards::OnDownloadLeaderboard);
	bLeaderboardDownloadRequested = false;
	return true;
}

void SteamUErLeaderboards::OnDownloadLeaderboard(LeaderboardScoresDownloaded_t *pCallback, bool bIOFailure)
{
	if (!bIOFailure)
	{
		m_nLeaderboardEntries = FMath::Min(pCallback->m_cEntryCount, k_nMaxLeaderboardEntries);

		for (int index = 0; index < m_nLeaderboardEntries; index++)
		{
			SteamUserStats()->GetDownloadedLeaderboardEntry(
				pCallback->m_hSteamLeaderboardEntries, index, &m_leaderboardEntries[index], NULL, 0);
		}
		bLeaderboardScoresFound = true;
		bLeaderboardScoresCached = true;
	}
}

bool SteamUErLeaderboards::DownloadScoresForUsers(TArray<CSteamID> usersToGet)
{
	if (!m_CurrentLeaderboard)
		return false;
	
	bLeaderboardUsersFound = false;

	if (usersToGet.Num() > 0)
	{
		requestedLeaderboardUserData.Empty();
		SteamAPICall_t hSteamAPICall = SteamUserStats()->DownloadLeaderboardEntriesForUsers(m_CurrentLeaderboard, &usersToGet[0], usersToGet.Num());
		m_callEntriesForUsers.Set(hSteamAPICall, this, &SteamUErLeaderboards::OnEntriesForUsers);
		return true;
	}
	return false;
}

void SteamUErLeaderboards::OnEntriesForUsers(LeaderboardScoresDownloaded_t *pResult, bool bIOFailure)
{
	if (!bIOFailure)
	{
		if (pResult->m_cEntryCount > 0)
		{
			requestedLeaderboardUserData.Empty();
			for (int index = 0; index < pResult->m_cEntryCount; index++)
			{
				LeaderboardEntry_t leaderboardEntry;
				SteamUserStats()->GetDownloadedLeaderboardEntry(pResult->m_hSteamLeaderboardEntries, index, &leaderboardEntry, NULL, 0);
				requestedLeaderboardUserData.Add(FSteamUErLeaderboardData(UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(leaderboardEntry.m_steamIDUser)),
					leaderboardEntry.m_nGlobalRank,	leaderboardEntry.m_nScore)
					);
			}
			bLeaderboardUsersFound = true;
		}
	}
}
#endif