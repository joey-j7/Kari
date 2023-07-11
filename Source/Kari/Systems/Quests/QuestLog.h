#pragma once

#include "CoreMinimal.h"

#include "TableComponent.h"
#include "UI/Quests/QuestWidget.h"

#include "Quest.h"
#include "Events/TaskEvent.h"

#include "QuestLog.generated.h"

class UDefaultGameInstance;
class UInteractableComponent;

UCLASS()
class KARI_API UQuestLog : public UTableComponent
{
	GENERATED_BODY()

public:
	UQuestLog();

	/* Get */
	UFUNCTION(BlueprintCallable, Category = QuestLog)
	TArray<FQuest> GetQuests() const;

	UFUNCTION(BlueprintCallable)
	void AddQuest(int32 QuestID, int32 CharacterID= -1);

	UFUNCTION(BlueprintCallable)
	void CompleteQuest(int32 QuestID);

	UFUNCTION(BlueprintCallable)
	void MarkQuestAsActive(int32 QuestID);

	UFUNCTION(BlueprintCallable)
	void ResetQuest(int32 QuestID);

	UFUNCTION(BlueprintCallable)
	void CompleteTask(int32 TaskID);

	UFUNCTION(BlueprintCallable)
	void AddRevisionIDChangeToTask(int32 TaskID, int32 CharacterID, int32 StoryID);

	// Freezes all task up until
	UFUNCTION(BlueprintCallable, Category = QuestLog)
	void FreezeTask(int32 TaskID);

	UFUNCTION(BlueprintCallable)
	void UpdateActiveTasks();

	UFUNCTION(BlueprintCallable)
	void OnInteractWithTask(UInteractableComponent* Interactable);

	void SetParentInstance(UDefaultGameInstance* I) { Instance = I; }

	UPROPERTY(BlueprintReadOnly)
	bool NPCQuestFlag = false;

	/* Saving & Loading */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool SaveData();
	void WriteData(FArchive &Ar);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool LoadData();
	void ReadData(FArchive &Ar);

	const FQuest* FindQuest(uint32 ID) const;
	const int32* FindCharacterID(uint32 ID) const;
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnQuestCompleted(FQuest Quest);

	UFUNCTION(BlueprintNativeEvent)
	void OnTaskEnter(FTask Task);

	UFUNCTION(BlueprintNativeEvent)
	void OnTaskExit(FTask Task);

	void UpdateTask(FTask& Task);

	UFUNCTION(BlueprintNativeEvent)
	void OnTaskCompleted(FTask Task);

	UPROPERTY(BlueprintReadOnly)
	UQuestWidget* Widget = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TMap<int32, FQuest> Quests;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ActiveQuests;

	// QuestId, CharacterId
	TMap<int32, int32> ActiveCharacterIds;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> CompletedQuests;

	UPROPERTY()
	TArray<FTaskEvent> Events;
	
	UDefaultGameInstance* Instance = nullptr;
};
