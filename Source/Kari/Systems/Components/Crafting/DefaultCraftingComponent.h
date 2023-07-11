

#pragma once

#include "Components/TableComponent.h"
#include "Systems/Interfaces/ICraftable.h"

#include "DefaultCraftingComponent.generated.h"

class UTexture2D;

class UCharacterComponent;
class UDefaultCraftingWidget;
class UDefaultGameInstance;

/**
 * @brief - DefaultCrafting Component.
 * This component holds basic information for both the
 * craft system of the merchant as for Kari.
 *
 * They both use similar objects
 */
UCLASS(Abstract)
class KARI_API UDefaultCraftingComponent : public UTableComponent, public IICraftable
{
	GENERATED_BODY()

	friend class ADefaultPlayer;
	friend class ADefaultMerchant;
public:
	UDefaultCraftingComponent();
	UDefaultCraftingComponent(const FString& name, const FString& dropdownColumn, const FString& linkedColumn = "", uint32 linkedId = UINT_MAX);

	void SetShown(bool show) { Shown = show; }
	bool IsShown() const { return Shown; }
	
	void BeginPlay() override;

	virtual void ShowMenu_Implementation() override;
	virtual void HideMenu_Implementation() override;

	virtual TArray<FCraftSlot> PopulateCraftableItems_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = Trade)
	TArray<FTradeSlot> PopulateItems() const;

	UFUNCTION(BlueprintCallable, Category = Trade)
	void SetNextDialogueId(int32 iNextDialogueId) { this->NextDialogueId = static_cast<uint32>(iNextDialogueId); }
protected:
	UPROPERTY()
	bool Shown = false;

	/**
	* @brief - Next Dialogue ID of the story component
	*/ 
	uint32 NextDialogueId = UINT_MAX;
	
	UCharacterComponent* CharacterComponent = nullptr;

	UDefaultGameInstance* DefaultGameInstance = nullptr;

	/**
	 * @brief Trade widget to show information about the items that can(not) be crafted
	 */
	TSubclassOf<class UWidgetBlueprintGeneratedClass> TradeWidgetClass;
	UDefaultCraftingWidget* TradeWidget = nullptr;
};