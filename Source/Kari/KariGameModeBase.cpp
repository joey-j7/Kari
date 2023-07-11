#include "KariGameModeBase.h"

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"
#include "Systems/Components/DialogueEventComponent.h"
#include "Systems/Components/Stats/DefaultStatComponent.h"
#include "Systems/Saving/LoadReasons.h"
#include "Systems/Traversal/Entrance.h"
#include "Systems/Saving/SaveLibrary.h"
#include "Systems/Revisioning/Revisioning.h"
#include "Systems/Quests/QuestLog.h"
#include "Systems/Inventory/Inventory.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

AKariGameModeBase::AKariGameModeBase()
{
}

void AKariGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());

	GameInstance->BeginPlay(World);

	UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ELevelLoadReason"), true);
	FString Reason = UGameplayStatics::ParseOption(OptionsString, "Reason");
	int64 EnumValue = Enum->GetValueByNameString(Reason);
	LoadLevelReason = EnumValue != -1 ? static_cast<ELevelLoadReason>(EnumValue) : ELevelLoadReason::None;

	switch (LoadLevelReason)
	{
	case ELevelLoadReason::Traversal:
	{	
		OnTraversal();
		break;
	}
	case ELevelLoadReason::Loading:
	{
		OnLoading();
		break;
	}
	case ELevelLoadReason::Respawning:
	{
		OnRespawning();
		break;
	}
	case ELevelLoadReason::NewGame:
	{
		OnNewGame();
		break;
	}
	default:
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No load actions were performed"));
		break;
	}
}

void AKariGameModeBase::EndPlay(EEndPlayReason::Type Reason)
{
	GameInstance->EndPlay(Reason);
}

void AKariGameModeBase::OnExit(FDialogueEvent dialogueEvent)
{
	this->DialogueEvent = dialogueEvent;
}

ELevelLoadReason AKariGameModeBase::GetLoadReason()
{
	UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ELevelLoadReason"), true);
	FString Reason = UGameplayStatics::ParseOption(OptionsString, "Reason");
	int64 EnumValue = Enum->GetValueByNameString(Reason);
	return EnumValue != -1 ? static_cast<ELevelLoadReason>(EnumValue) : ELevelLoadReason::None;
}

void AKariGameModeBase::OnNewGame_Implementation()
{
	UWorld* World = GetWorld();
	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

	if (Player)
	{
		Player->GameBegin();
	}

	// Update persistent data
	GameInstance->QuestLog->UpdateActiveTasks();
	GameInstance->Revisioning->UpdateCharacterRevisions();
	
	USaveLibrary::SaveGame(World);
}

void AKariGameModeBase::OnRespawning_Implementation()
{
	UWorld* World = GetWorld();
	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

	FString ClearInventory = UGameplayStatics::ParseOption(OptionsString, "ClearInventory");
	// Remove items from inventory if they aren't found in a safe spot
	if (ClearInventory.ToBool())
	{
		GameInstance->Inventory->Respawn();
	}

	// Loads the player's last saved position
	Player->LoadData(false);
	// Clear status effects
	Player->Respawn();

	// Update persistent data
	GameInstance->QuestLog->UpdateActiveTasks();
	GameInstance->Revisioning->UpdateCharacterRevisions();

	// Load level-specific data
	USaveLibrary::LoadGatesData(World, true);
	USaveLibrary::LoadDroppedItemsData(World, true);

	// Save the game so that you can't evade penalty resulted by the player's death
	USaveLibrary::SaveGame(World);
}

void AKariGameModeBase::OnLoading_Implementation()
{
	UWorld* World = GetWorld();
	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));
	// Load data
	GameInstance->LoadPersistentData();
	GameInstance->Inventory->LoadData(false);
	GameInstance->QuestLog->LoadData();
	GameInstance->Revisioning->LoadData();
	Player->LoadData(false);
	Player->GetBlessing()->LoadData(Player, false);

	// Update persistent data
	GameInstance->QuestLog->UpdateActiveTasks();
	GameInstance->Revisioning->UpdateCharacterRevisions();

	// Load level-specific data
	USaveLibrary::LoadGatesData(World);
	USaveLibrary::LoadDroppedItemsData(World);

	// Load discovered levels
	GameInstance->VisitedLevels.Empty();
	GameInstance->DiscoveredLevels.Empty();
	USaveLibrary::DeletePersistent();
	USaveLibrary::LoadDiscovered(World);
}

void AKariGameModeBase::OnTraversal_Implementation()
{
	UWorld* World = GetWorld();
	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

	Player->LoadData(true);
	Player->GetBlessing()->LoadData(Player, true);
	GameInstance->Inventory->LoadData(true);

	TArray<AActor*> Entrances;
	UGameplayStatics::GetAllActorsOfClass(World, TSubclassOf<AEntrance>(AEntrance::StaticClass()), Entrances);
	FString EntranceName = UGameplayStatics::ParseOption(OptionsString, "Entrance");
	// Loop over all entrances
	for (auto Actor : Entrances)
	{
		AEntrance* Entrance = Cast<AEntrance>(Actor);

		// If the entrance name equals that one set for traversal, place Kari at the entrance
		if (Entrance->Name == EntranceName)
		{
			Player->SetActorLocation(Entrance->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
			Player->GetCameraSystem()->ResetArmLocation();
			break;
		}
	}

	GameInstance->QuestLog->UpdateActiveTasks();
	GameInstance->Revisioning->UpdateCharacterRevisions();

	// Load per-level specific data
	USaveLibrary::LoadGatesData(World, true);
	USaveLibrary::LoadDroppedItemsData(World, true);

}



