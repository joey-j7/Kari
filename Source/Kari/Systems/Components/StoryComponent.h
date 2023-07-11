#pragma once

#include <string>

#include "CoreMinimal.h"

#include "Components/TableComponent.h"
#include "Events/DialogueEvent.h"
#include "Events/Audio/BGMFader.h"

#include <utility>

#include "StoryComponent.generated.h"

class UDialogueWidget;
class UQuestWidget;
class ADefaultPlayer;

class UDefaultGameInstance;
class UAudioManager;
class UPlaylist;

class UCharacterComponent;

/**
 * 
 */
UCLASS(ClassGroup = (Database), meta = (BlueprintSpawnableComponent))
class KARI_API UStoryComponent : public UTableComponent
{
	GENERATED_BODY()
	friend class ADefaultNPC;
	friend class ADialogueInteractable;

public:
	UStoryComponent();

	void BeginPlay() override;

	// This story ID will be applied after the dialogue is done,
	// to prevent conflicts with story id's being reset due to dialogue progression
	UFUNCTION(BlueprintCallable)
	void SetPendingStoryID(int32 StoryID);

	UFUNCTION(BlueprintCallable)
	void AddPendingQuestID(int32 QuestID);

	UFUNCTION(BlueprintCallable)
	bool ShowDialogue(int32 dialogueId = -1);
	void HideDialogue();
	
	void Advance();

	// Check for next dialogue options
	bool CheckNext();

	UDialogueWidget* GetDialogueWidget() const { return DialogueWidget; }

protected:
	void SetStory(int32 storyId, int32 dialogueId = -INT_MAX);
	void SetDialogue(int32 dialogueId);

	bool CheckDialogueOptions();

	TSubclassOf<class UWidgetBlueprintGeneratedClass> DialogueWidgetClass;
	UDialogueWidget* DialogueWidget = nullptr;

	UDefaultGameInstance* GameInstance = nullptr;
	UAudioManager* AudioManager = nullptr;

	UCharacterComponent* CharacterComponent = nullptr;

	int32 PendingStoryID = -INT32_MAX;

	UPROPERTY()
	TArray<int32> PendingQuestIDS;

	UPROPERTY()
	TArray<FDialogueEvent> Events;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDialogueWidget> CustomWidgetOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool MusicFaderActive = false;

	UPROPERTY()
	UBGMFader* MusicFader;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	FString BGMPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float FadeTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	bool RevertOnOverlapEnd = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	bool ResetOnExit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	bool Loop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	bool Shuffle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float Volume = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	int32 LoopStart = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	int32 LoopEnd = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float PitchMin = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float PitchMax = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGMFader")
	float Delay = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGMFader")
	bool Shown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ShouldCameraZoom = true;

	int32 CurrentID = -INT_MAX;
	int32 NextID = -INT_MAX;
};
