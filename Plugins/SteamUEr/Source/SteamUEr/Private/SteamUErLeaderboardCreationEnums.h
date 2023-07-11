#pragma once


UENUM(BlueprintType)
enum class ESteamUErLeaderboardSortMethodEnum : uint8
{
	None,
	Ascending,
	Descending
};

UENUM(BlueprintType)
enum class ESteamUErLeaderboardTypeEnum : uint8
{
	None,
	Numeric,
	TimeSeconds,
	TimeMilliSeconds
};

