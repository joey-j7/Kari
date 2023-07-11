#include "DefaultGameInstance.h"

#include "Serialization/BufferArchive.h"
#include "Misc/FileHelper.h"

#include "GameFramework/WorldSettings.h"

#include "UI/HUD/ActionLog/ActionLogWidget.h"
#include "UI/HUD/HUDWidget.h"
#include "UI/HUD/Interact/HUDInteractWidget.h"

#include "Systems/Inventory/Inventory.h"
#include "Systems/Quests/QuestLog.h"
#include "Systems/Revisioning/Revisioning.h"
#include "Systems/Audio/AudioManager.h"
#include "Systems/Time/TimeManager.h"

#include "Ticker.h"

#include "Kismet/GameplayStatics.h"


UDefaultGameInstance::UDefaultGameInstance() : TimerManager(this)
{
#if PLATFORM_IOS || PLATFORM_ANDROID
	bIsMobile = true;
#endif
}

void UDefaultGameInstance::BeginPlay(UWorld* World)
{
	AudioManager->BeginPlay(World);
	Inventory->InitCustomTick(World);

	if (!TimeManager)
		TimeManager = NewObject<UTimeManager>();

	TimeManager->Init();
}

void UDefaultGameInstance::EndPlay(EEndPlayReason::Type Reason)
{
	if (HUDLog)
	{
		HUDLog->RemoveFromViewport();
		HUDLog = nullptr;
	}

	if (HUDInteract)
	{
		HUDInteract->RemoveFromViewport();
		HUDInteract = nullptr;
	}

	if (HUD)
	{
		HUD->RemoveFromViewport();
		HUD = nullptr;
	}
}

void UDefaultGameInstance::Init()
{
	// Register delegate for ticker callback
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UDefaultGameInstance::TickDelegate));

	Revisioning = NewObject<URevisioning>();

	Inventory = NewObject<UInventory>();
	Inventory->SetParentInstance(this);

	QuestLog = NewObject<UQuestLog>();
	QuestLog->SetParentInstance(this);

	AudioManager = NewObject<UAudioManager>();
	AudioManager->SetParentInstance(this);

	if (!TimeManager)
	TimeManager = NewObject<UTimeManager>();

	if (!GConfig->GetFloat(TEXT("PostProcessing"), TEXT("Gamma"), Gamma, GGameUserSettingsIni))
	{
		GConfig->SetFloat(TEXT("PostProcessing"), TEXT("Gamma"), Gamma, GGameUserSettingsIni);
		GConfig->Flush(false);
	}

	Super::Init();
}

void UDefaultGameInstance::Shutdown()
{
	// Unregister ticker delegate
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);

	Super::Shutdown();
}

bool UDefaultGameInstance::TickDelegate(float DeltaSeconds)
{
	Tick(DeltaSeconds);
	return true;
}

void UDefaultGameInstance::Tick_Implementation(float DeltaSeconds)
{
	if (GetWorld()->IsPaused())
		return;

	TimeManager->Update(DeltaSeconds);
	AudioManager->Tick(DeltaSeconds);
}

void UDefaultGameInstance::AddMoney(int32 Amount)
{
	if (Amount <= 0)
		return;

	Money += Amount;

	if (GetHUDLog())
		GetHUDLog()->AddMessage(-1, "You obtained " + FString::FromInt(Amount) + " coins!");
}

bool UDefaultGameInstance::RemoveMoney(int32 Amount, bool EnsureSufficiency)
{
	if ((EnsureSufficiency && Amount > Money) || Amount < 0)
		return false;

	Money -= Amount;

	return true;
}

void UDefaultGameInstance::EmptyMoney()
{
	Money = 0;
}

UActionLogWidget* UDefaultGameInstance::GetHUDLog()
{
	if (!HUDLog)
	{
		const FStringClassReference WidgetReference(TEXT("/Game/UI/HUD/ActionLog/WBP_HUD_ActionLog.WBP_HUD_ActionLog_C"));
		UClass* WidgetClass = WidgetReference.TryLoadClass<UActionLogWidget>();

		if (WidgetClass)
		{
			HUDLog = CreateWidget<UActionLogWidget>(
                UGameplayStatics::GetPlayerController(GetWorld(), 0),
                WidgetClass
            );

			if (HUDLog)
				HUDLog->AddToViewport(99);
		}
	}

	return HUDLog;
}

UHUDInteractWidget* UDefaultGameInstance::GetHUDInteract()
{
	if (!HUDInteract)
	{
		const FStringClassReference WidgetReference(TEXT("/Game/UI/HUD/Interact/WBP_HUDInteract.WBP_HUDInteract_C"));
		UClass* WidgetClass = WidgetReference.TryLoadClass<UHUDInteractWidget>();

		if (WidgetClass)
		{
			HUDInteract = CreateWidget<UHUDInteractWidget>(
				UGameplayStatics::GetPlayerController(GetWorld(), 0),
				WidgetClass
				);

			if (HUDInteract)
				HUDInteract->AddToViewport(99);
		}
	}

	return HUDInteract;
}

UHUDWidget* UDefaultGameInstance::GetHUD()
{
	if (!HUD)
	{
		UWorld* World = GetWorld();

		if (!World)
			return nullptr;

		const FStringClassReference WidgetReference(TEXT("/Game/UI/HUD/WBP_HUD.WBP_HUD_C"));
		UClass* WidgetClass = WidgetReference.TryLoadClass<UHUDWidget>();

		if (WidgetClass)
		{
			HUD = CreateWidget<UHUDWidget>(
				UGameplayStatics::GetPlayerController(World, 0),
				WidgetClass
				);
		}
	}

	return HUD;
}

void UDefaultGameInstance::SetGameSpeed(float Speed)
{
	GetWorld()->GetWorldSettings()->SetTimeDilation(Speed);

	GameSpeed = Speed;
	TimeManager->SetTimeMultiplier(Speed);
}

void UDefaultGameInstance::SaveGamma()
{
	GConfig->SetFloat(TEXT("PostProcessing"), TEXT("Gamma"), Gamma, GGameUserSettingsIni);
	GConfig->Flush(false);
}

void UDefaultGameInstance::Reset()
{
	GhostPassAccess = false;
	SunBeamReflection = false;
	MistRadiusMultiplier = 1.0f;
	MistDamageMultiplier = 0.0f;
	
	Revisioning = NewObject<URevisioning>();

	Inventory = NewObject<UInventory>();
	Inventory->SetParentInstance(this);

	QuestLog = NewObject<UQuestLog>();
	QuestLog->SetParentInstance(this);

	TimeManager = NewObject<UTimeManager>();

	VisitedLevels.Empty();
	DiscoveredLevels.Empty();
}

bool UDefaultGameInstance::SavePersistentData()
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WritePersistentData(ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\PersistentSavedData.sav"));

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}

void UDefaultGameInstance::WritePersistentData(FArchive &Ar)
{
	Ar << GhostPassAccess;
	Ar << SunBeamReflection;
	Ar << MistRadiusMultiplier;
	Ar << MistDamageMultiplier;

	// Save time of day
	int32 Hours = TimeManager->GetDate().GetHour();
	int32 Minutes = TimeManager->GetDate().GetMinute();
	int32 Seconds = TimeManager->GetDate().GetSecond();
	Ar << Hours;
	Ar << Minutes;
	Ar << Seconds;

	Ar << IsStormActive;
}

bool UDefaultGameInstance::LoadPersistentData()
{
	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\PersistentSavedData.sav"))) return false;

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadPersistentData(FromBinary);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void UDefaultGameInstance::ReadPersistentData(FArchive &Ar)
{
	Ar << GhostPassAccess;
	Ar << SunBeamReflection;
	Ar << MistRadiusMultiplier;
	Ar << MistDamageMultiplier;

	// Load time of day
	int32 Hours, Minutes, Seconds;
	Ar << Hours;
	Ar << Minutes;
	Ar << Seconds;

	Ar << IsStormActive;

	TimeManager->SetTime(Hours, Minutes, Seconds);
}
