#pragma once
#include "GameFramework/CheatManager.h"
#include "KariCheatManager.generated.h"

UCLASS()
class UKariCheatManager : public UCheatManager
{

	GENERATED_BODY()

public:
	UFUNCTION(Exec, Category = ConsoleCommands)
	void KariConsoleTest();

	UFUNCTION(Exec, Category = ConsoleCommands)
	void KariAddDefaultDrain();

	UFUNCTION(Exec, Category = ConsoleCommands)
	void KariPrintSaveVersion();

	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ConsoleCommands)
	void KariConsoleTestBP();

	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ConsoleCommands)
	void ShowTime();

	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ConsoleCommands)
	void ChangeTime(int32 hour, int32 minutes, int32 seconds);

	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ConsoleCommands)
	void UnlockGate();

	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ConsoleCommands)
	void AddItem(int32 ID);
};