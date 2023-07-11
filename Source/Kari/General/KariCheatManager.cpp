#include "KariCheatManager.h"

#include "Characters/Playables/DefaultPlayer.h"
#include "Systems/Saving/SaveLibrary.h"
#include "Kismet/GameplayStatics.h"

void UKariCheatManager::KariConsoleTest()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("This is an on screen message!"));
}

void UKariCheatManager::KariAddDefaultDrain()
{
	UWorld* World = GetWorld();
	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

	if (Player)
	{
		Player->GameBegin();
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Added default blessing drain. Warning: might stack if applied multiple times."));
	}
}

void UKariCheatManager::KariPrintSaveVersion()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Save libary version: %d"), USaveLibrary::SaveDataVersioning));
}