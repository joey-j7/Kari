#pragma once

#include "GameFramework/Actor.h"
#include "Systems/Components/InteractableComponent.h"
#include "Engine/EngineTypes.h"

#include "RestorePoint.generated.h"

UCLASS(ClassGroup = (Gameplay))
class KARI_API ARestorePoint : public AActor
{
	GENERATED_BODY()

public:
	ARestorePoint();
	void BeginPlay() override;

	UFUNCTION()
	virtual void OnInteract_Implementation();

protected:
	UPROPERTY(EditAnywhere)
	FComponentReference StatReference;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RestorePoint")
	UInteractableComponent* InteractableComponent;
};