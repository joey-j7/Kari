

#pragma once
#include <cstdio>
#include "CoreMinimal.h"

#include "TableComponent.h"
#include "GameFramework/Actor.h"

#include "Item.h"

#include "Systems/Components/InteractableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

#include "ItemBase.generated.h"

/**
 *
 */
UCLASS()
class KARI_API AItemBase : public AActor
{
	GENERATED_BODY()

public:
	AItemBase();

	void BeginPlay() override;
	UTableComponent* GetTableComponent() const { return TableComponent; };

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void OnItemUse(bool& CanUse_out);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void OnItemEquippedTick(float DeltaTime, bool& CanUse_out);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void OnItemPickedUp(bool& CanPickUp_out);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void OnItemDropped(bool& CanDrop_out);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Item")
	void OnItemSaved();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Item")
	void OnItemLoaded();

	UFUNCTION(BlueprintCallable, Category = "Item", meta = (DisplayName = "Get Item ID"))
	int32 GetID();

	UFUNCTION(BlueprintCallable, Category = "Item", meta = (DisplayName = "Set Item ID"))
	void SetID(int32 id);

	UFUNCTION(BlueprintCallable, Category = "Item", meta = (DisplayName = "Get Item Properties"))
	FItem GetItem() const { return Item; };

	UFUNCTION(BlueprintCallable, Category = "Item", meta = (DisplayName = "Get Item Icon"))
	UTexture2D* GetIcon() const { return ItemIcon; }

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = ItemBase)
	bool IsEquipped = false;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadWrite, Category = ItemBase)
	float Timer = 0.0f;

protected:
	void SetEntity();

	UFUNCTION()
	virtual void OnInteract_Implementation();

#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ItemBase)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = ItemBase)
	UBoxComponent* BoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemBase)
	UTableComponent* TableComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemBase)
	UInteractableComponent* InteractableComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database Properties")
	UTexture2D* ItemIcon;

	UPROPERTY(SaveGame)
	FItem Item;
};