#pragma once

#include "CoreMinimal.h"

#include "Components/TableComponent.h"

class UStoryComponent;
class UDefaultCraftingComponent;

#include "CharacterComponent.generated.h"

UCLASS(ClassGroup = (Database), meta = (BlueprintSpawnableComponent))
class KARI_API UCharacterComponent : public UTableComponent
{
	GENERATED_BODY()
	friend class ADefaultNPC;
	friend class ADefaultMerchant;
	friend class ADialogueInteractable;

public:
	UCharacterComponent();

	void BeginPlay() override;
	void BeginDestroy() override;

	void OnTableIDChanged() override;

	UFUNCTION()
	UStoryComponent* GetStoryComponent() const { return StoryComponent; }
	
	UFUNCTION()
	UDefaultCraftingComponent* GetTradeComponent() const { return TradeComponent; }

protected:
	bool Shown = false;

	UStoryComponent* StoryComponent = nullptr;
	UDefaultCraftingComponent* TradeComponent = nullptr;

	uint32 CurrentID = UINT_MAX;
	uint32 NextID = UINT_MAX;
};
