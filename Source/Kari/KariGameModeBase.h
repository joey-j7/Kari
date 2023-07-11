

#pragma once

#include "CoreMinimal.h"
#include "ObjectMacros.h"
#include "GameFramework/GameMode.h"
#include "General/DefaultGameInstance.h"
#include "Events/DialogueEvent.h"
#include <Systems/Saving/LoadReasons.h>
#include "KariGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKariGameModeDelegate_OnTimeChanged, FDialogueEvent, DialogueEvent);

/**
 * 
 */
UCLASS()
class KARI_API AKariGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	AKariGameModeBase();
	
	void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type Reason) override;
	
	UFUNCTION(BlueprintCallable, Category = "Kari Game Mode")
	void OnExit(FDialogueEvent dialogueEvent);

	UPROPERTY(BlueprintAssignable, Category = "Kari Game Mode")
	FKariGameModeDelegate_OnTimeChanged OnTimeChanged;

	UFUNCTION(BlueprintCallable, Category = "Kari Game Mode")
	ELevelLoadReason GetLoadReason();

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Kari Game Mode")
	void OnTraversal();

	UFUNCTION(BlueprintNativeEvent, Category = "Kari Game Mode")
	void OnLoading();

	UFUNCTION(BlueprintNativeEvent, Category = "Kari Game Mode")
	void OnRespawning();

	UFUNCTION(BlueprintNativeEvent, Category = "Kari Game Mode")
	void OnNewGame();

	UPROPERTY(BlueprintReadWrite, Category = "Kari Game Mode")
	bool DayNightOption = false;

	UPROPERTY(BlueprintReadOnly, Category = "Kari Game Mode")
	UDefaultGameInstance* GameInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Kari Game Mode")
	FDialogueEvent DialogueEvent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ELevelLoadReason LoadLevelReason = ELevelLoadReason::None;
};
