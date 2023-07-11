

#pragma once

#include <string>

#include "DefaultCraftingComponent.h"
#include "TradeComponent.generated.h"

/**
 * @brief - this class is purely to not get a nullptr on the tradecomponent
 * in the merchant. This is engine thing. don't know where it comes from.
 */
UCLASS(ClassGroup = (Database), meta = (BlueprintSpawnableComponent))
class KARI_API UTradeComponent : public UDefaultCraftingComponent
{
	GENERATED_BODY()
	
	// friend class ADefaultPlayer;
public:
	UTradeComponent();
};
