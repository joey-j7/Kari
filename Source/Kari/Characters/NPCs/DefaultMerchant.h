#pragma once

#include "CoreMinimal.h"
#include "DefaultNPC.h"

#include "DefaultMerchant.generated.h"

class UDefaultCraftingComponent;
class UCraftingComponent;

/**
 *
 */
UCLASS()
class KARI_API ADefaultMerchant : public ADefaultNPC
{
	friend class URevisioning;

	GENERATED_BODY()

public:
	ADefaultMerchant();

	void OnInteract_Implementation() override;
	void OnInteractEnd_Implementation() override;

	void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void Tick(float DeltaSeconds) override;

	UDefaultCraftingComponent* GetTradeComponent() const;

	int32 GetMoney() const { return Money; }
	int32 GetRevisionID() const { return RevisionID; }

protected:
	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UDefaultCraftingComponent* TradeComponent = nullptr;

	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 Money = 0;

	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 RevisionID = 0;
};
