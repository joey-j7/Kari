// This code royally borrows from the Spacewar example, slightly edited
//========= Copyright © 1996-2009, Valve LLC, All rights reserved. ============

#pragma once

UENUM(BlueprintType)
enum class ESteamUErScoreTypeEnum : uint8
{
	Global UMETA(DisplayName = "Global"),
	AroundUser UMETA(DisplayName = "Global around user"),
	Friends UMETA(DisplayName = "Friends")
};

#if PLATFORM_WINDOWS
#pragma push_macro("ARRAY_COUNT")	// duplicate macro definitions in the steam api and the UE api, so we need to do this first
#undef ARRAY_COUNT
#include "ThirdParty/Steamworks/Steamv147/sdk/public/steam/steam_api.h"
#pragma pop_macro("ARRAY_COUNT")
#include "Engine.h"
#include "SteamUErLeaderboardData.h"
#include "SteamUErLeaderboardCreationData.h"

class SteamUErLeaderboards
{

public:
	int k_nMaxLeaderboardEntries = 10;									// maximum number of leaderboard entries we can display

	LeaderboardEntry_t* m_leaderboardEntries;  // leaderboard entries we received from DownloadLeaderboardEntries
	int m_nLeaderboardEntries;											// number of leaderboard entries we received

	SteamUErLeaderboards(FString leaderboardName, int inLeaderboardEntries = 10);
	~SteamUErLeaderboards() {
		delete[] myLeaderboardName;
		delete[] m_leaderboardEntries;
	};

	void FindLeaderboard();
	void FindOrCreateLeaderboard(FSteamUErLeaderboardCreationData& inputData);
	bool UploadScore(int score, bool switchToForceUpdate = false);
	bool UploadWaitingScore();
	bool waitingScoreUploadForce;

	bool DownloadScores(int NumberOfScores, ESteamUErScoreTypeEnum leaderboardType, int startingIndex = 0); // number of requested scores, different types of data
	bool DownloadScoresWaiting();

	bool LeaderboardInited();
	bool bLeaderboardInitRequested = false;
	bool bLeaderboardFound = false;

	bool bLeaderboardScoresFound = false;
	
	bool bLeaderboardScoreWaiting = false; // we called upload before the leaderboard was found, we'll keep trying in the update loop
	int waitingScore = 0;

	bool bLeaderboardDownloadRequested = false;
	int waitingDownloadNum = 0;
	int waitingLeaderboardStart = 0;
	ESteamUErScoreTypeEnum cachedLeaderboardType;
	bool bLeaderboardScoresCached = false;


	bool bLeaderboardUsersFound = false;
	TArray<FSteamUErLeaderboardData> requestedLeaderboardUserData;
	bool DownloadScoresForUsers(TArray<CSteamID> usersToGet);
	void OnEntriesForUsers(LeaderboardScoresDownloaded_t *pResult, bool bIOFailure);
	CCallResult<SteamUErLeaderboards, LeaderboardScoresDownloaded_t> m_callEntriesForUsers;

	void OnFindLeaderboard(LeaderboardFindResult_t *pResult, bool bIOFailure);
	CCallResult<SteamUErLeaderboards, LeaderboardFindResult_t> m_callResultFindLeaderboard;
	void OnUploadScore(LeaderboardScoreUploaded_t *pResult, bool bIOFailure);
	CCallResult<SteamUErLeaderboards, LeaderboardScoreUploaded_t> m_callResultUploadScore;
	void OnDownloadLeaderboard(LeaderboardScoresDownloaded_t *pResult, bool bIOFailure);
	CCallResult<SteamUErLeaderboards, LeaderboardScoresDownloaded_t> m_callResultDownloadScores;
	
private:
	SteamLeaderboard_t m_CurrentLeaderboard; // Handle to leaderboard
	char* myLeaderboardName; // leaderboard copy
};
#endif