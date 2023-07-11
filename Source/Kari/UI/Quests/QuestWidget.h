

#pragma once

#include "CoreMinimal.h"
#include "UI/NavigatableWidget.h"

#include "QuestWidget.generated.h"

class UStoryComponent;

/**
 *
 */
UCLASS()
class KARI_API UQuestWidget : public UNavigatableWidget
{
	GENERATED_BODY()

public:
	void NativeConstruct() override;

	void SetQuest(uint32 CurrentID, uint32 acceptID, uint32 denyID);
	void SetStoryComponent(const UStoryComponent& StoryComponent);

	void OnCancel_Implementation() override;

protected:
	void OnInventory() {};
	void OnPause() {};

	UFUNCTION(BlueprintImplementableEvent, Category = Dialogue)
	void OnValuesSet(const FString& title, const FString& description, int32 expReward, int32 moneyReward);

	void OnNavY_Implementation(float Value) override;

	UPROPERTY(BlueprintReadWrite, Category = Inventory)
	int32 CurrentQuestID = 0;

	int32 SelectedQuestID = 0;
	
	uint32 ID = UINT_MAX;
	UStoryComponent* StoryComponent = nullptr;
};