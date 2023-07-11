#include "PersistenceLibrary.h"

#include "Engine.h"
#include "Engine/World.h"
#include "General/DefaultGameInstance.h"

float UPersistenceLibrary::GetMistRadiusMultiplier(UObject * WorldContextObject)
{
	// Access the default game instance
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	auto In = World->GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	return Instance->MistRadiusMultiplier;
}

float UPersistenceLibrary::GetMistDamageMultiplier(UObject * WorldContextObject)
{
	// Access the default game instance
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	auto In = World->GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	return Instance->MistDamageMultiplier;
}

void UPersistenceLibrary::SetMistRadiusMultiplier(UObject * WorldContextObject, float Multiplier)
{
	// Access the default game instance
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	auto In = World->GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	Instance->MistRadiusMultiplier = Multiplier;
}

void UPersistenceLibrary::SetMistDamageMultiplier(UObject * WorldContextObject, float Multiplier)
{
	// Access the default game instance
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	auto In = World->GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	Instance->MistDamageMultiplier = Multiplier;
}

bool UPersistenceLibrary::GetGhostPassAccess(UObject* WorldContextObject){
	// Access the default game instance
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	auto In = World->GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	return Instance->GhostPassAccess;
}

void UPersistenceLibrary::SetGhostPassAccess(UObject* WorldContextObject, bool access) {
	// Access the default game instance
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	auto In = World->GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	Instance->GhostPassAccess = access;
}

bool UPersistenceLibrary::GetSunBeamReflection(UObject* WorldContextObject)
{
	// Access the default game instance
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	auto In = World->GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	return Instance->SunBeamReflection;
}

void UPersistenceLibrary::SetSunBeamReflection(UObject* WorldContextObject, bool reflect)
{
	// Access the default game instance
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	auto In = World->GetGameInstance();
	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(In);

	Instance->SunBeamReflection = reflect;
}