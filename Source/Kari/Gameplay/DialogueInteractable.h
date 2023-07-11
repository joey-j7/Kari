#pragma once
#include "CoreMinimal.h"
#include "Characters/NPCs/DefaultNPC.h"
#include "Systems/Components/InteractableComponent.h"

#include "DialogueInteractable.generated.h"

UCLASS(ClassGroup = (Gameplay))
class KARI_API ADialogueInteractable : public AActor
{
GENERATED_BODY()

public:
	ADialogueInteractable();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void OnInteract_Implementation();

	UFUNCTION()
	virtual void OnInteractEnd_Implementation();

	UCharacterComponent* GetCharacterComponent() const { return CharacterComponent; }
	UStoryComponent* GetStoryComponent() const { return StoryComponent; }

protected:
	void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Category = ItemBase, VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCharacterComponent* CharacterComponent = nullptr;

	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStoryComponent* StoryComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = NPCCharacter)
	UInteractableComponent* InteractableComponent;

	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsInteracting = false;

	UDefaultGameInstance* GameInstance = nullptr;
};
