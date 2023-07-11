
 
#pragma once
 
#include <string>
 
#include "DefaultCraftingComponent.h"
#include "CraftingComponent.generated.h"
 
/**
 * 
 */
UCLASS(ClassGroup = (Database), meta = (BlueprintSpawnableComponent))
class KARI_API UCraftingComponent : public UDefaultCraftingComponent
{
	GENERATED_BODY()
	
	// friend class ADefaultPlayer;
public:
	UCraftingComponent();
 
	UFUNCTION(BlueprintCallable, Category = Trade)
	TArray<FCraftSlot> PopulateCraftableItems_Implementation() override;
};