

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Environment/Props/Item.h"

#include "ICraftable.generated.h"

// TODO should probably be named to something better
USTRUCT(BlueprintType)
struct FTradeSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Trade Slot")
	FItem Item;

	UPROPERTY(BlueprintReadOnly, Category = "Trade Slot")
	int32 Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Trade Slot")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FCraftRequirement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Craft Requirements")
	FItem Item;

	UPROPERTY(BlueprintReadOnly, Category = "Craft Requirements")
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct FCraftSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Craft Slot")
	FItem Craftable;

	UPROPERTY(BlueprintReadOnly, Category = "Craft Slot")
	TArray<FCraftRequirement> Requirements;

	UPROPERTY(BlueprintReadOnly, Category = "Craft Slot")
	int32 Value = 0;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UICraftable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class KARI_API IICraftable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//Behaviour of the trade and crafting components
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Crafting")
	TArray<FCraftSlot> PopulateCraftableItems();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Crafting")
	void HideMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Crafting")
	void ShowMenu();
};
