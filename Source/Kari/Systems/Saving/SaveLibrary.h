#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoadReasons.h"

#include "SaveLibrary.generated.h"

// Predeclaration of classes
class ADefaultPlayer;
class UDefaultGameInstance;

UCLASS()
class KARI_API USaveLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	friend class AKariGameModeBase;
public:
	USaveLibrary() = default;

	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save/Load", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true", BlueprintInternalUseOnly = "true"))
	UFUNCTION(BlueprintCallable, Category = "Save/Load", meta = (WorldContext = "WorldContextObject"))
	static void SaveGame(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Save/Load", meta = (WorldContext = "WorldContextObject"))
	static void LoadGame(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Save/Load", meta = (WorldContext = "WorldContextObject"))
	static void Respawn(UObject* WorldContextObject, bool ClearInventory = true);

	UFUNCTION(BlueprintCallable, Category = "Save/Load", meta = (WorldContext = "WorldContextObject"))
	static void Traverse(UObject* WorldContextObject, FName Name, FString Entrance, bool FadeAudio = true);

	UFUNCTION(BlueprintCallable, Category = "Save/Load", meta = (WorldContext = "WorldContextObject"))
	static void NewGame(UObject* WorldContextObject, FName Level);

	UFUNCTION(BlueprintCallable, Category = "Save/Load")
	static bool SaveExists();

	UFUNCTION(BlueprintCallable, Category = "Save/Load")
	static void DeleteSave();

	UFUNCTION(BlueprintCallable, Category = "Save/Load")
	static void DeletePersistent();

	// LoadTxtFileToString
	UFUNCTION(BlueprintCallable, Category = "Utils")
	static FString LoadTxtFileToString(FString filename);

	static const int SaveDataVersioning = 10;

private:
	static bool SaveLevelData(UWorld* WorldObject);
	static void WriteLevelData(UWorld* WorldObject, FArchive &Ar);
	static bool LoadLevelData(UWorld* WorldObject, ELevelLoadReason LoadReason = ELevelLoadReason::Loading);
	static bool LoadLevelData(UWorld* WorldObject, bool& HasTraversalOccured, ELevelLoadReason LoadReason = ELevelLoadReason::Loading);
	static void ReadLevelData(UWorld* WorldObject, FArchive &Ar, bool& HasTraversalOccured, ELevelLoadReason LoadReason = ELevelLoadReason::Loading);

	static bool SaveGatesData(UWorld* WorldObject, bool persistent = false);
	static void WriteGatesData(UWorld* WorldObject, FArchive &Ar);
	static bool LoadGatesData(UWorld* WorldObject, bool persistent = false);
	static void ReadGatesData(UWorld* WorldObject, FArchive &Ar);

	static bool SaveDroppedItemsData(UWorld* WorldObject, bool Persistent = false);
	static void WriteDroppedItemsData(UWorld* WorldObject, FArchive &Ar);
	static bool LoadDroppedItemsData(UWorld* WorldObject, bool Persistent = false);
	static void ReadDroppedItemsData(UWorld* WorldObject, FArchive &Ar);

	static bool SaveVersioning();
	static void WriteVersioning(FArchive &Ar);
	static bool LoadVersioning(int& Version);
	static void ReadVersioning(FArchive &Ar, int& Version);

	static bool SaveDiscovered(UWorld* WorldObject);
	static void WriteDiscovered(UWorld* WorldObject, FArchive &Ar);
	static bool LoadDiscovered(UWorld* WorldObject);
	static void ReadDiscovered(UWorld* WorldObject, FArchive &Ar);

	static ADefaultPlayer* GetPlayer();
	static UDefaultGameInstance* GetInstance(UWorld* WorldObject);
};