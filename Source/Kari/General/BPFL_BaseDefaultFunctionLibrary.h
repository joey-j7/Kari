// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPFL_BaseDefaultFunctionLibrary.generated.h"

class UPlaylist;

/**
 * 
 */
UCLASS()
class KARI_API UBPFL_BaseDefaultFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	* @brief - Load assets from a specific folder inside game/content.
	* @param PathFromContentFolder - Path to start the search from.
	* @param AssetClass - which asset class should we load.
	* @param LoadFromSubfolders - to search inside sub folders as well
	*/
	UFUNCTION(BlueprintCallable, Category = "Default|Utilities")
    static TArray<UObject*> DynamicLoadContentFromPath(FString PathFromContentFolder = "", UClass* AssetClass = nullptr, bool LoadFromSubfolders = false);

	/**
	 * @brief - Amend a character before an other certain character
	 * @param Str - String that needs to be checked
	 * @param Character - Character that should be added.
	 * @param SkipIndex - The index from where the adding should start.
	 * @optional - TODO maybe add if we should amend at the front or back.
	 */
	static FString AmendSentence(const FString& Str, const char* Character, int32 SkipIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	static void FadeAudio(UPlaylist* Playlist, float FadeSpeed);
};
