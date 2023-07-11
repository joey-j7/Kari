#pragma once
#include "SteamUErLeaderboardData.generated.h"


USTRUCT(BlueprintType)
struct FSteamUErLeaderboardData
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player name")
	FString playerName = "---";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rank")
	int32 ranking = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Value")  // "score" in Spacewar, but feels silly for 'time' values or whatever else you want to store
	int32 value = 0;

	FSteamUErLeaderboardData()
	{

	}

	FSteamUErLeaderboardData(FString inName, int32 inRank, int32 inValue)
	{
		playerName = inName;
		ranking = inRank;
		value = inValue;
	}
};
