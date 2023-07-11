

#pragma once

#include "CoreMinimal.h"

#include "Characters/DefaultCharacter.h"

#include "Systems/Components/InteractableComponent.h"
#include "Systems/Components/CharacterComponent.h"

#include "DefaultNPC.generated.h"

/**
 *
 */
UCLASS()
class KARI_API ADefaultNPC : public ADefaultCharacter
{
	GENERATED_BODY()
public:
	ADefaultNPC();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool ShouldRotateToPlayer() const { return RotateToPlayer; }

	UFUNCTION(BlueprintCallable, Category = NPCCharacter)
	virtual void OnInteract_Implementation();

	UFUNCTION(BlueprintCallable, Category = NPCCharacter)
	virtual void OnInteractEnd_Implementation();

	virtual void Tick(float DeltaSeconds) override;

	UCharacterComponent* GetCharacterComponent() const { return CharacterComponent; }
	UStoryComponent* GetStoryComponent() const { return StoryComponent; }

protected:
	void BeginPlay() override;

	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCharacterComponent* CharacterComponent = nullptr;
	
	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStoryComponent* StoryComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = NPCCharacter)
	UInteractableComponent* InteractableComponent;

	UPROPERTY(Category = NPCCharacter, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool RotateToPlayer = true;

	UPROPERTY(Category = NPCCharacter, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsInteracting = false;

	float RotationSpeed = 7.f;

	UPROPERTY()
	FTimerHandle RotationTimerHandle;

	void InteractRotateImpl(float TimerRate);
	void InteractRotateImpl();
};
