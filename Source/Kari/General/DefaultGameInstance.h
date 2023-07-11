#pragma once
#include "Engine/GameInstance.h"

#include "TimerManager.h"
#include "Environment/Props/Item.h"
#include "DefaultGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDefaultGameInstance_OnLostFocus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDefaultGameInstance_OnReceivedFocus);

class ADefaultPlayerController;
class AItemBase;
class UHUDWidget;
class UActionLogWidget;
class UHUDInteractWidget;
class URevisioning;
class UAudioManager;
class UInventory;
class UQuestLog;
class UTimeManager;

UCLASS()
class KARI_API UDefaultGameInstance : public UGameInstance
{
	friend class USaveLibrary;
	friend class UItemFunctionLibrary;
	friend class AKariGameModeBase;
	friend class UPersistenceLibrary;
	friend class UHUDWidget;

	GENERATED_BODY()

public:
	UDefaultGameInstance();
	
	void BeginPlay(UWorld* World);
	void EndPlay(EEndPlayReason::Type Reason);

	/* Currency */
	UFUNCTION(BlueprintCallable)
	void AddMoney(int32 Amount);

	UFUNCTION(BlueprintCallable)
	bool RemoveMoney(int32 Amount, bool EnsureSufficiency = true);

	UFUNCTION(BlueprintCallable)
	void EmptyMoney();

	UFUNCTION(BlueprintCallable)
	int32 GetMoney() { return Money; }

	UFUNCTION(BlueprintCallable)
	UActionLogWidget* GetHUDLog();

	UFUNCTION(BlueprintCallable)
	UHUDInteractWidget* GetHUDInteract();

	UFUNCTION(BlueprintCallable)
	UHUDWidget* GetHUD();
	
	UFUNCTION(BlueprintCallable)
	void SetGameSpeed(float Speed);

	UFUNCTION(BlueprintCallable)
	float GetGameSpeed() const { return GameSpeed; };

	UFUNCTION(BlueprintCallable)
	void SaveGamma();

	UFUNCTION(BlueprintCallable)
	void Reset();

	/* Systems */
	UPROPERTY(BlueprintReadOnly)
	URevisioning* Revisioning = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UAudioManager* AudioManager = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UInventory* Inventory = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UQuestLog* QuestLog = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UTimeManager* TimeManager = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float Gamma = 0.5f;

	FTimerManager TimerManager;

	UPROPERTY(BlueprintAssignable, Category = "GameInstance")
	FDefaultGameInstance_OnLostFocus OnLostFocus;

	UPROPERTY(BlueprintAssignable, Category = "GameInstance")
	FDefaultGameInstance_OnReceivedFocus OnReceivedFocus;

protected:
	void Init() override;
	void Shutdown() override;

	bool TickDelegate(float DeltaSeconds);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameInstance")
	void Tick(float DeltaSeconds);

	/* Saving/Loading */
	bool SavePersistentData();
	void WritePersistentData(FArchive &Ar);
	bool LoadPersistentData();
	void ReadPersistentData(FArchive &Ar);

	FDelegateHandle TickDelegateHandle;

	/* Game Variables */
	UPROPERTY(BlueprintReadOnly)
	bool HasController = false; // TODO: Assign
	float GameSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0))
	int32 Money = 0;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsMobile = false;
	
	/* UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHUDWidget* HUD = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UActionLogWidget* HUDLog = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHUDInteractWidget* HUDInteract = nullptr;

	/* Persistent variables */
	UPROPERTY(BlueprintReadWrite)
	bool IsStormActive = false;

	bool GhostPassAccess = false;
	bool SunBeamReflection = false;
	float MistRadiusMultiplier = 1.0f;
	float MistDamageMultiplier = 0.0f;
	TSet<FString> VisitedLevels; // Visited since last save
	TSet<FString> DiscoveredLevels; // All levels the player ever visited
};