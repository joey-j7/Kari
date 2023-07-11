// 2017 Turfster
// This basically just interfaces with the Steam API, filling in gaps left by the OnlineSubsystemSteam
// (which you need to have installed if you want to make changes, as per https://docs.unrealengine.com/latest/INT/Programming/Online/Steam/)
// PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.
#pragma once

#include "Engine.h"
#include "SteamUErManager.h"
#include "SteamUErLeaderboardData.h"
#include "SteamUErLeaderboardCreationData.h"
#include "SteamUErLeaderboards.h"
#include "SteamUErGlobalStats.h"
#include "SteamUErBPLibrary.generated.h"

UCLASS()
class USteamUErBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

		~USteamUErBPLibrary()
		{
#if PLATFORM_WINDOWS
			UE_LOG(LogTemp, Log, TEXT("SteamUEr: Time to die.")); // clean up after ourselves
			delete myGlobalStatsObject;
#endif
		}

#if PLATFORM_WINDOWS
		static SteamUErGlobalStats* myGlobalStatsObject;
		static ISteamUserStats* myStats;
		static ISteamUser* myUser;
		static bool inShutdownMode;
#endif

		// Call me to open a single leaderboard. Use at the start of your program.
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Init leaderboard", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErInitLeaderboard(FString leaderboardName, int numLeaderboardItems = 10);

		// Call me to open various leaderboards. Use at the start of your program.
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Init leaderboards", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErInitLeaderboards(TArray<FString> leaderboardNames, int numLeaderboardItems = 10);

		// Call me to open *or create* various leaderboards. Use at the start of your program.
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Init or create leaderboards", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErInitOrCreateLeaderboards(TArray<FSteamUErLeaderboardCreationData> leaderboardCreationList);

		// Close down and clean up your leaderboards. Use when shutting down your program.
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Close leaderboards", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErCloseLeaderboards();

		// Has Steam given the OK after init on this leaderboard?
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Steam ready to return leaderboard data?", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErGetLeaderboardInited(FString leaderboardName);

		// Call to make Steam handle your requests and see if anything returned yet
		// Needs to be called (and probably looped) after every call to the Steam backend to upload/get data
		// @param scoreSetWaiting					  Has a leaderboard upload request been completed? 
		// @param leaderboardGetWaiting				  Is the requested data for one or more of your leaderboards ready?
		// @param userLeaderboardGetWaiting			  Is the data for one or more specific user leaderboard lookups ready?
		// @param globalStatsWaiting				  Has Steam readied global stat data for lookup?
		// @param globalPlayerCountWaiting			  Has Steam returned the current global player count?
		// @param globalAchievementPercentagesWaiting -- currently not implemented --
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is steam data ready for me?", Keywords = "Leaderboards"), Category = "SteamUEr")
		static bool SteamUErPingAsynchronous(bool& scoreSetWaiting, bool& leaderboardGetWaiting, bool& userLeaderboardGetWaiting, bool& globalStatsWaiting, bool& globalPlayerCountWaiting, bool& globalAchievementPercentagesWaiting);

		// Send Steam a score upload request for a specified leaderboard.
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Upload score to leaderboard", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErUploadScoreToLeaderboard(FString leaderboardName, int score, bool useForceUpdate = false);

		// If "Is steam data ready for me?" returned true for leaderboardGetwaiting and you have multiple leaderboards, use this to check which one is ready
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is leaderboard data ready?", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErGetLeaderboardReady(FString leaderboardName);

		// If "Is steam data ready for me?" returned true for leaderboardGetWaiting, call this function to get the actual leaderboard data.
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get leaderboard data from leaderboard", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErGetLeaderboard(FString leaderboardName, TArray<FSteamUErLeaderboardData>& leaderboardData);

		// Returns an array of Texture2D with player icons for the currently waiting leaderboard
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get leaderboard player icons", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static TArray<UTexture2D*> SteamUErGetLeaderboardAvatars(FString leaderboardName);

		// Request numScores values from the named leaderboard data with leaderboardType (Global, Global around user, Friends)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request leaderboard", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards")
		static bool SteamUErRequestLeaderboard(FString leaderboardName, int numScores, ESteamUErScoreTypeEnum leaderboardType, int startingIndex = 0);

		// Request leaderboard information to match the *users* of another leaderboard
		// @param leaderboardName				The leaderboard you want to get new information from
		// @param userNameSourceLeaderboard		The leaderboard you want to use as data source for the user names
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request leaderboard user information", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards|User data")
		static bool SteamUErRequestLeaderboardFromLeaderboard(FString leaderboardName, FString userNameSourceLeaderboard);

		// Is the requested *user* information for this leaderboard available yet?
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Leaderboard user information available?", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards|User data")
		static bool SteamUErUserLeaderboardDataReady(FString leaderboardName);

		// Get the requested *user* information for this leaderboard
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get requested specific user data", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards|User data")
		static bool SteamUErGetUserLeaderboardData(FString leaderboardName, TArray<FSteamUErLeaderboardData>& leaderboardData);

		// Request leaderboard information for the player
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request leaderboard player information", Keywords = "Leaderboards"), Category = "SteamUEr|Leaderboards|User data")
		static bool SteamUErRequestPlayerFromLeaderboard(FString leaderboardName);

		// Are we connected to Steam?
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Are we connected to Steam?", Keywords = "Steam"), Category = "SteamUEr")
		static bool SteamUErIsSteamConnected();


		UFUNCTION(BlueprintCallable, meta = (DisplayName = "DateTime to string", Keywords = "Generic"), Category = "Turfster")
		static FString SteamUErConvertDateTimeToString(FDateTime unlockTimestamp);

		// Get current world type
		// -1	Something is wrong
		// 0    None
		// 1	Game world
		// 2	Editor edited world
		// 3	PIE world
		// 4	Editor Preview tool world
		// 5	Game Preview world
		// 6	Inactive
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Unreal Engine status", Keywords = "Generic"), Category = "Turfster")
		static int SteamUErGetPlayMode(UObject* WorldContextObject);


		// Returns a Texture2D with the Player's Icon
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get player avatar", Keywords = "Steam"), Category = "SteamUEr")
		static UTexture2D* SteamUErGetPlayerAvatar();


		// Returns a Texture2D with the achievement icon that matches the *current* state (locked/unlocked)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get achievement icon", Keywords = "Achievement"), Category = "SteamUEr|Achievement")
		static UTexture2D* SteamUErGetAchievementIcon(FString achievementName);

		// Returns information about a named achievement by identifier
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get achievement display attributes (name, desc, hidden)", Keywords = "Achievement"), Category = "SteamUEr|Achievement")
		static bool SteamUErGetAchievementDisplayAttribute(FString achievementName, FString& localisedName, FString& localisedDescription, bool& isHidden);

		// Did player unlock achievement, and when if they did?
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get achievement unlock status", Keywords = "Achievement"), Category = "SteamUEr|Achievement")
		static bool SteamUErGetAchievementUnLockedTime(FString achievementName, bool& unlockStatus, FDateTime& unlockTimestamp);

		// Give player achievement
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Give achievement", Keywords = "Achievement"), Category = "SteamUEr|Achievement")
		static bool SteamUErSetAchievement(FString achievementName);

		// Did player unlock achievement
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Does player have achievement", Keywords = "Achievement"), Category = "SteamUEr|Achievement")
		static bool SteamUErGetAchievement(FString achievementName);

		// Lock achievement
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear achievement", Keywords = "Achievement"), Category = "SteamUEr|Achievement")
		static bool SteamUErClearAchievement(FString achievementName);

		// How many achievements does the game have
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get number of achievements", Keywords = "Achievement"), Category = "SteamUEr|Achievement")
		static int SteamUErNumAchievements();

		// Get the identifier for achievement nr X
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get achievement #X identifier", Keywords = "Achievement"), Category = "SteamUEr|Achievement")
		static FString SteamUErGetAchievementName(int32 achievementNumber);

		// Reset all stats to zero (and optionally lock all achievements again)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Reset all stats (and optionally achievements)", Keywords = "Achievement, Stat" ), Category = "SteamUEr")
		static bool SteamUErResetAllStats(bool achievementsToo);


		// Set stat value (integer)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set stat value (int)", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErSetStatInt(FString statName, int32 value);

		// Set stat value (float)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set stat value (float)", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErSetStatFloat(FString statName, float value);

		// Get stat value (integer)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get stat value (int)", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static int32 SteamUErGetStatInt(FString statName);

		// Get stat value (integer) with a success/fail result
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get stat value (int) with success state", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErGetStatIntBool(FString statName, int32& value);

		// Get stat value (float)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get stat value (float)", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static float SteamUErGetStatFloat(FString statName);

		// Get stat value (float) with a success/fail result
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get stat value (float) with success state", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErGetStatFloatBool(FString statName, float& value);


		// Request global stats for the last numDays. This takes a while!
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request global stats", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErRequestGlobalStats(int numDays);

		// Request global current player count. This takes a while!
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request current player count", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErRequestCurrentPlayers();

		// Request global achievement percentages. This takes a while!
/*		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request global achievement percentages", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErRequestAchievementPercentages();*/

		// Are the requested global achievement percentages ready yet?
		// Poll this before getting the actual player count data!
		// (or use the generic "is steam data ready for me?" pins)
/*		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Global achievement percentages ready?", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErGlobalAchievementPercentagesReady();*/

		// Is the requested global player count ready yet? 
		// Poll this before getting the actual player count data!
		// (or use the generic "is steam data ready for me?" pins)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Current player count ready?", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErCurrentPlayersReady();

		// Get current player count
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get current player count", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static int SteamUErCurrentPlayers();

		// Is the requested global stat data ready yet?
		// Poll this before getting the actual player count data!
		// (or use the generic "is steam data ready for me?" pins)
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Global stats ready?", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErGlobalStatsReady();

		// Done getting your global stats? Reset the readiness so the readiness isn't triggered any more
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Reset global stats readiness", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static void SteamUErInvalidateGlobalStats();

		// Get global stat value (64 bit integer) with a success/fail result
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get global stat int64 value as a string with success state", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErGetGlobalStatIntBool(FString statName, FString& value);

		// Get global stat value (float) with a success/fail result
		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get global stat float value with success state", Keywords = "Stat"), Category = "SteamUEr|Stats")
		static bool SteamUErGetGlobalStatFloatBool(FString statName, float& value);

};
