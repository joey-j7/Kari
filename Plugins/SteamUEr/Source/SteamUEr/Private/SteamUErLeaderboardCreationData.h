#pragma once
#include "SteamUErLeaderboardCreationEnums.h"
#include "SteamUErLeaderboardCreationData.generated.h"


USTRUCT(BlueprintType)
struct FSteamUErLeaderboardCreationData
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leaderboard name")
	FString leaderboardName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leaderboard list count")
	int leaderboardItems = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leaderboard sort method")
	ESteamUErLeaderboardSortMethodEnum leaderboardSortMethod = ESteamUErLeaderboardSortMethodEnum::Descending;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leaderboard data type")
	ESteamUErLeaderboardTypeEnum leaderboardType = ESteamUErLeaderboardTypeEnum::Numeric;
};
