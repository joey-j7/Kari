#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Engine/Texture2D.h"
#include "Item.generated.h"

struct FTableRow;

class AItemBase;

UENUM(BlueprintType)
enum class EEffectType : uint8
{
	E_NONE = 0 UMETA(DisplayName = "None"),
	E_RESTORE_BLESSING = 1 UMETA(DisplayName = "Restore Blessing"),
	E_RESISTANCE_TO_BLESSING_DRAIN_DURING_DAY = 2 UMETA(DisplayName = "Resistance To Blessing Drain During The Day"),
	E_RESISTANCE_TO_BLESSING_DAMAGE = 3 UMETA(DisplayName = "Resistance To Blessing Damage"),
	E_GIVE_ITEM = 4 UMETA(DisplayName = "Give Item"),
	E_RESPAWN = 5 UMETA(DisplayName = "Respawn"),
	E_WISP_HAZARD_RESISTANCE = 6 UMETA(DisplayName = "Wisp Hazard Resistance"),
	E_MUSHROOM_HAZARD_RESISTANCE = 7 UMETA(DisplayName = "Mushroom Hazard Resistance"),
	E_MIST_HAZARD_RESISTANCE = 8 UMETA(DisplayName = "Mist Hazard Resistance"),
	E_RESTORE_BLESSING_LOST = 9 UMETA(DisplayName = "Restore Blessing Lost"),
	E_PATH_HAZARD_REVEAL = 10 UMETA(DisplayName = "Path Hazard Reveal"),
	E_REFLECT_SUN_BEAM = 11 UMETA(DisplayName = "Reflect Sun Beam"),
	E_Light_TORCH = 12 UMETA(DisplayName = "Light Torch"),
	E_Lure_WISP = 13 UMETA(Displayname = "Lure Wisp"),
	E_UNDEFINED = 14 UMETA(DisplayName = "UNDEFINED")
};

USTRUCT(BlueprintType)
struct FItemEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Item Effect")
	EEffectType EffectType = EEffectType::E_UNDEFINED;

	UPROPERTY(BlueprintReadOnly, Category = "Item Effect")
	float PrimaryValue = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Item Effect")
	float SecondaryValue = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Item Effect")
	float Time = 0.f;

	void SaveLoad(FArchive &Ar)
	{
		Ar << EffectType; //int8
		Ar << PrimaryValue; //float
		Ar << SecondaryValue; //float
		Ar << Time; //float
	}
};

USTRUCT(BlueprintType)
struct FItem
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 ID = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	FString Description = "";

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	FString Name = "";

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	bool Despawnable = true;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	bool Droppable = true;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	bool Equippable = false;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	bool UseOnPickup = false;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 SellValue = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	bool Sellable = true;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 StackLimit = 99;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	FItemEffect Effect;

	static FItem ConvertRow(const FTableRow& Row, int32 ID);
	void SaveLoad(FArchive &Ar);

	UPROPERTY()
	FString ClassPath;
};