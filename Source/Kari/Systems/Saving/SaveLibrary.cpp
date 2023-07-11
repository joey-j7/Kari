#include "SaveLibrary.h"

#include <cassert>

#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Serialization/BufferArchive.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"

#include "Events/DefaultEvent.h"
#include "Characters/Playables/DefaultPlayer.h"
#include "Systems/Components/Stats/DefaultStatComponent.h"
#include "Systems/Revisioning/Revisioning.h"
#include "Systems/Quests/QuestLog.h"
#include "Systems/Inventory/Inventory.h"
#include "Environment/Props/ItemBase.h"
#include "Gameplay/Gate.h"

#include "General/DefaultGameInstance.h"
#include "KariGameModeBase.h"

void USaveLibrary::SaveGame(UObject* WorldContextObject)
{
	// Pls don't delete this is reference code for me
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%i"), PCarr.Num()));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, World->GetName());
	
	// Get world
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	
	// Save level data
	USaveLibrary::SaveLevelData(World);

	// Access the default game instance
	UDefaultGameInstance* Instance = GetInstance(World);

	// Get the player
	ADefaultPlayer* Player = GetPlayer();

	// Save data
	Instance->SavePersistentData();
	Instance->Inventory->SaveData(false);
	Instance->QuestLog->SaveData();
	Instance->Revisioning->SaveData();

	// Menus don't have (normal) players.
	if (Player)
	{
		Player->SaveData(false);
		Player->GetBlessing()->SaveData(false);
	}

	// Save level-specific data
	SaveGatesData(World);
	SaveDroppedItemsData(World);
	Instance->DiscoveredLevels.Add(World->GetName());

	// Save data of prior levels
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	Instance->VisitedLevels.Remove(World->GetName()); // The data of this level is already saved
	for (const auto& LevelName : Instance->VisitedLevels)
	{
		FString Source = "Persistence\\" + LevelName;
		FString Destination = "Saving\\" + LevelName;

		PlatformFile.DeleteDirectoryRecursively(*Destination);
		PlatformFile.CopyDirectoryTree(*Destination, *Source, true);

		Instance->DiscoveredLevels.Add(LevelName);
	}
	Instance->VisitedLevels.Empty();
	USaveLibrary::SaveDiscovered(World);

	// Save the versioning
	USaveLibrary::SaveVersioning();
}

void USaveLibrary::LoadGame(UObject* WorldContextObject)
{
	// Get world
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	// Access the default game instance
	UDefaultGameInstance* Instance = GetInstance(World);

	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\LevelSavedData.sav")))
	{
		check(false);
	}

	if (BinaryArray.Num() <= 0)
	{
		check(false);
	}

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);

	FString Level;
	FromBinary << Level;

	TArray< FStringFormatArg > Args;
	Args.Add(FStringFormatArg(UEnum::GetValueAsString(ELevelLoadReason::Loading)));

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	// Load level
	UGameplayStatics::OpenLevel(World, FName(*Level), true, FString::Format(TEXT("?Reason={0}"), Args));
}

void USaveLibrary::Respawn(UObject* WorldContextObject, bool ClearInventory)
{
	// Get world
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	// Access the default game instance
	UDefaultGameInstance* Instance = GetInstance(World);

	// Save persistent data
	SaveGatesData(World, true);
	SaveDroppedItemsData(World, true);

	// Add the level to levels to save
	Instance->VisitedLevels.Add(World->GetName());

	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\LevelSavedData.sav")))
	{
		check(false);
	}

	if (BinaryArray.Num() <= 0)
	{
		check(false);
	}

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);

	FString Level;
	FromBinary << Level;

	TArray< FStringFormatArg > Args;
	Args.Add(FStringFormatArg(UEnum::GetValueAsString(ELevelLoadReason::Respawning)));
	Args.Add(FStringFormatArg(ClearInventory));

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	// Load level
	UGameplayStatics::OpenLevel(World, FName(*Level), true, FString::Format(TEXT("?Reason={0}?ClearInventory={1}"), Args));
}

void USaveLibrary::Traverse(UObject* WorldContextObject, FName Name, FString Entrance, bool FadeAudio)
{
	// Get world
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	
	// Access the default game instance
	UDefaultGameInstance* Instance = GetInstance(World);

	// Add the level to levels to save
	Instance->VisitedLevels.Add(World->GetName());

	// Get the player
	ADefaultPlayer* Player = GetPlayer();

	// Perform traversal actions on the player
	Player->SaveData(true);
	Player->GetBlessing()->SaveData(true);
	Instance->Inventory->SaveData(true);

	// Save level-specific data
	SaveGatesData(World, true);
	SaveDroppedItemsData(World, true);

	TArray< FStringFormatArg > Args;
	Args.Add(FStringFormatArg(UEnum::GetValueAsString(ELevelLoadReason::Traversal)));
	Args.Add(FStringFormatArg(Entrance));

	UGameplayStatics::OpenLevel(World, Name, true, FString::Format(TEXT("?Reason={0}?Entrance={1}"), Args));
}

void USaveLibrary::NewGame(UObject* WorldContextObject, FName Level) {
	// Remove previous saved game from disk
	USaveLibrary::DeleteSave();
	
	// Get world
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	
	// Access the default game instance
	UDefaultGameInstance* Instance = GetInstance(World);

	// Reset to default values
	Instance->IsStormActive = false;
	Instance->GhostPassAccess = false;
	Instance->SunBeamReflection = false;
	Instance->MistRadiusMultiplier = 1.0f;
	Instance->MistDamageMultiplier = 0.0f;

	TArray< FStringFormatArg > Args;
	Args.Add(FStringFormatArg(UEnum::GetValueAsString(ELevelLoadReason::NewGame)));

	// Load level
	UGameplayStatics::OpenLevel(World, Level, true, FString::Format(TEXT("?Reason={0}"), Args));
}

bool USaveLibrary::SaveExists()
{
	if (FPaths::FileExists(TEXT("Saving\\LevelSavedData.sav")))
	{
		int Version;
		USaveLibrary::LoadVersioning(Version);

		if (USaveLibrary::SaveDataVersioning == Version) {
			return true;
		}
	}
	return false;
}
void USaveLibrary::DeleteSave()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	PlatformFile.DeleteDirectoryRecursively(TEXT("Saving"));
	PlatformFile.DeleteDirectoryRecursively(TEXT("Persistence"));
}
void USaveLibrary::DeletePersistent()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	PlatformFile.DeleteDirectoryRecursively(TEXT("Persistence"));
}

FString USaveLibrary::LoadTxtFileToString(FString Filename) {
	auto Directory{ FPaths::ProjectContentDir() };
	FString Result{};
	auto& File{ FPlatformFileManager::Get().GetPlatformFile() };
	if (File.CreateDirectory(*Directory)) {
		auto Path{ Directory + "/"+ Filename };
		FFileHelper::LoadFileToString(Result, *Path);
	}
	return Result;
}

bool USaveLibrary::SaveLevelData(UWorld* WorldObject)
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WriteLevelData(WorldObject, ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\LevelSavedData.sav"));

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}
void USaveLibrary::WriteLevelData(UWorld* WorldObject, FArchive &Ar)
{
	FString level = WorldObject->GetName();
	Ar << level;
}

bool USaveLibrary::LoadLevelData(UWorld* WorldObject, ELevelLoadReason LoadReason)
{
	bool DummyRef;
	return LoadLevelData(WorldObject, DummyRef, LoadReason);
}
bool USaveLibrary::LoadLevelData(UWorld* WorldObject, bool& HasTraversalOccured, ELevelLoadReason LoadReason)
{
	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\LevelSavedData.sav"))) return false;

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadLevelData(WorldObject, FromBinary, HasTraversalOccured, LoadReason);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void USaveLibrary::ReadLevelData(UWorld* WorldObject, FArchive &Ar, bool& HasTraversalOccured, ELevelLoadReason LoadReason)
{
	// Access the default game instance
	UDefaultGameInstance* Instance = GetInstance(WorldObject);

	FString Level;
	Ar << Level;

	TArray< FStringFormatArg > Args;
	Args.Add(FStringFormatArg(UEnum::GetValueAsString(LoadReason)));

	// Load level
	UGameplayStatics::OpenLevel(WorldObject, FName(*Level), true, FString::Format(TEXT("?Reason={0}"), Args));
}

bool USaveLibrary::SaveGatesData(UWorld* WorldObject, bool persistent)
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WriteGatesData(WorldObject, ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	// Calculate file name
	FString level = WorldObject->GetName();
	if (persistent) {
		level = "Persistence\\" + level + "\\GatesSavedData.sav";
	}
	else {
		level = "Saving\\" + level + "\\GatesSavedData.sav";
	}

	//Save binaries to disk
	bool result = FFileHelper::SaveArrayToFile(ToBinary, *level);

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}
void USaveLibrary::WriteGatesData(UWorld* WorldObject, FArchive &Ar)
{
	// Find the gates
	TSubclassOf<AGate> classToFind;
	classToFind = AGate::StaticClass();
	TArray<AActor*> foundGates;
	UGameplayStatics::GetAllActorsOfClass(WorldObject, classToFind, foundGates);

	// Write the gates' data to the save file
	int dummyInt = foundGates.Num();
	Ar << dummyInt;

	for (int i = 0; i < foundGates.Num(); i++) {
		AGate* gate = Cast<AGate>(foundGates[i]);

		FString dummyString = gate->GetHumanReadableName();
		Ar << dummyString;
		bool dummyBool = gate->IsLocked();
		Ar << dummyBool;
	}
}

bool USaveLibrary::LoadGatesData(UWorld* WorldObject, bool persistent)
{
	TArray<uint8> BinaryArray;

	// Calculate file name
	FString level = WorldObject->GetName();
	if (persistent) {
		level = "Persistence\\" + level + "\\GatesSavedData.sav";
	}
	else {
		level = "Saving\\" + level + "\\GatesSavedData.sav";
	}

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, *level)) return false;

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadGatesData(WorldObject, FromBinary);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void USaveLibrary::ReadGatesData(UWorld* WorldObject, FArchive &Ar)
{
	// Write the gates' data to a map
	TMap<FString, bool> gatesData;
	int count;
	Ar << count;

	for (int i = 0; i < count; i++) {
		FString name;
		Ar << name;
		bool locked;
		Ar << locked;

		gatesData.Add(name, locked);
	}

	// Find the gates
	TSubclassOf<AGate> classToFind;
	classToFind = AGate::StaticClass();
	TArray<AActor*> foundGates;
	UGameplayStatics::GetAllActorsOfClass(WorldObject, classToFind, foundGates);

	// Apply the data to the gates
	for (int i = 0; i < foundGates.Num(); i++) {
		AGate* gate = Cast<AGate>(foundGates[i]);
		FString name = gate->GetHumanReadableName();
		bool* b = gatesData.Find(name);

		if (b && !*b) {
			gate->OnLoadUnlock();
		}
	}
}

bool USaveLibrary::SaveDroppedItemsData(UWorld * WorldObject, bool Persistent)
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WriteDroppedItemsData(WorldObject, ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	// Calculate file name
	FString Level = WorldObject->GetName();
	if (Persistent) {
		Level = "Persistence\\" + Level + "\\DroppedItemSavedData.sav";
	}
	else {
		Level = "Saving\\" + Level + "\\DroppedItemSavedData.sav";
	}

	//Save binaries to disk
	bool result = FFileHelper::SaveArrayToFile(ToBinary, *Level);

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}
void USaveLibrary::WriteDroppedItemsData(UWorld * WorldObject, FArchive & Ar)
{
	// Find the items
	TSubclassOf<AItemBase> ClassToFind = AItemBase::StaticClass();
	TArray<AActor*> FoundItemBases;
	UGameplayStatics::GetAllActorsOfClass(WorldObject, ClassToFind, FoundItemBases);

	// Count all non-despawnable items
	int FoundItemBasesCount = 0; 
	for (int i = 0; i < FoundItemBases.Num(); i++)
	{
		AItemBase* ItemBase = Cast<AItemBase>(FoundItemBases[i]);
		FItem Item = ItemBase->GetItem();

		if (!Item.Despawnable)
			FoundItemBasesCount++;
	}
	Ar << FoundItemBasesCount;

	// Write the items data to the save file
	for (int i = 0; i < FoundItemBases.Num(); i++) 
	{
		AItemBase* ItemBase = Cast<AItemBase>(FoundItemBases[i]);
		FItem Item = ItemBase->GetItem();

		if (Item.Despawnable)
			continue;

		// Save item
		int32 ItemID = Item.ID;
		Ar << ItemID;

		// Save transform
		FTransform Transform = ItemBase->GetTransform();

		float LocationX = Transform.GetLocation().X;
		float LocationY = Transform.GetLocation().Y;
		float LocationZ = Transform.GetLocation().Z;

		float RotationX = Transform.GetRotation().X;
		float RotationY = Transform.GetRotation().Y;
		float RotationZ = Transform.GetRotation().Z;
		float RotationW = Transform.GetRotation().W;

		float ScaleX = Transform.GetScale3D().X;
		float ScaleY = Transform.GetScale3D().Y;
		float ScaleZ = Transform.GetScale3D().Z;

		Ar << LocationX;
		Ar << LocationY;
		Ar << LocationZ;

		Ar << RotationX;
		Ar << RotationY;
		Ar << RotationZ;
		Ar << RotationW;

		Ar << ScaleX;
		Ar << ScaleY;
		Ar << ScaleZ;
	}
}

bool USaveLibrary::LoadDroppedItemsData(UWorld * WorldObject, bool Persistent)
{
	TArray<uint8> BinaryArray;

	// Calculate file name
	FString Level = WorldObject->GetName();
	if (Persistent) 
	{
		Level = "Persistence\\" + Level + "\\DroppedItemSavedData.sav";
	}
	else 
	{
		Level = "Saving\\" + Level + "\\DroppedItemSavedData.sav";
	}

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, *Level)) return false;

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadDroppedItemsData(WorldObject, FromBinary);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void USaveLibrary::ReadDroppedItemsData(UWorld * WorldObject, FArchive & Ar)
{
	UDefaultGameInstance* Instance = GetInstance(WorldObject);

	// Read the items array
	int ItemBasesCount;
	Ar << ItemBasesCount;

	for (int i = 0; i < ItemBasesCount; i++)
	{
		// Read item
		int32 ItemID;
		Ar << ItemID;

		// Save transform
		FTransform Transform = FTransform::Identity;

		float LocationX, LocationY, LocationZ;
		float RotationX, RotationY, RotationZ, RotationW;
		float ScaleX, ScaleY, ScaleZ;

		Ar << LocationX;
		Ar << LocationY;
		Ar << LocationZ;

		Ar << RotationX;
		Ar << RotationY;
		Ar << RotationZ;
		Ar << RotationW;

		Ar << ScaleX;
		Ar << ScaleY;
		Ar << ScaleZ;

		Transform.SetComponents(FQuat(RotationX, RotationY, RotationZ, RotationW), FVector(LocationX, LocationY, LocationZ), FVector(ScaleX, ScaleY, ScaleZ));

		// Spawn item
		//TODO
		//AItemBase& ActorRef = AItemBase::SpawnItemBlueprintDeferred(Instance, ItemID);
		//ActorRef.IsEquipped = false;

		//UGameplayStatics::FinishSpawningActor(&ActorRef, Transform);
	}
}

bool USaveLibrary::SaveVersioning()
{
	//Save the data to binary
	FBufferArchive ToBinary;
	USaveLibrary::WriteVersioning(ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\SaveVersioning.sav"));

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}
void USaveLibrary::WriteVersioning(FArchive &Ar)
{
	int Version = USaveLibrary::SaveDataVersioning;
	Ar << Version;
}

bool USaveLibrary::LoadVersioning(int& Version)
{
	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\SaveVersioning.sav"))) return false;

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadVersioning(FromBinary, Version);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void USaveLibrary::ReadVersioning(FArchive &Ar, int& Version)
{
	Ar << Version;
}

bool USaveLibrary::SaveDiscovered(UWorld* WorldObject)
{
	//Save the data to binary
	FBufferArchive ToBinary;
	USaveLibrary::WriteDiscovered(WorldObject, ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\SaveDiscovered.sav"));

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}
void USaveLibrary::WriteDiscovered(UWorld* WorldObject, FArchive &Ar)
{
	// Access the default game instance
	UDefaultGameInstance* Instance = GetInstance(WorldObject);

	// Get the amount of discovered levels
	int Amount = Instance->DiscoveredLevels.Num();
	Ar << Amount;

	// Write level names
	for (const auto& LevelName : Instance->DiscoveredLevels)
	{
		FString DummyString = LevelName;
		Ar << DummyString;
	}
}

bool USaveLibrary::LoadDiscovered(UWorld* WorldObject)
{
	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\SaveDiscovered.sav"))) return false;

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadDiscovered(WorldObject, FromBinary);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void USaveLibrary::ReadDiscovered(UWorld* WorldObject, FArchive &Ar)
{
	// Access the default game instance
	UDefaultGameInstance* Instance = GetInstance(WorldObject);

	// Get the platform file manager
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Get the amount of discovered levels
	int Amount;
	Ar << Amount;

	// Load level data into the persistence folder
	PlatformFile.CreateDirectory(TEXT("Persistence\\"));
	for (int i=0; i<Amount; i++)
	{
		FString LevelName;
		Ar << LevelName;
		Instance->DiscoveredLevels.Add(LevelName);

		FString Source = "Saving\\" + LevelName;
		FString Destination = "Persistence\\" + LevelName;
		PlatformFile.CopyDirectoryTree(*Destination, *Source, true);
	}
}

ADefaultPlayer* USaveLibrary::GetPlayer() {
	// Get player controller
	TArray<APlayerController*> PCarr;
	GEngine->GetAllLocalPlayerControllers(PCarr);

	if (PCarr.Num() == 0 || PCarr[0] == nullptr)
		return nullptr;
	
	AController* controller = Cast<AController>(PCarr[0]);
	APawn* playerPawn = controller->GetPawn();
	ADefaultPlayer* player = Cast<ADefaultPlayer>(playerPawn);

	return player;
}

UDefaultGameInstance* USaveLibrary::GetInstance(UWorld* WorldObject){
	auto in = WorldObject->GetGameInstance();
	UDefaultGameInstance* instance = Cast<UDefaultGameInstance>(in);

	ensure(instance);

	return instance;
}