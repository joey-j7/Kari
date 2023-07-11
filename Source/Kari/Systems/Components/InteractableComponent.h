#pragma once
#include "CoreMinimal.h"
#include "Components/TableComponent.h"
#include "InteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractableEventDelegate_OnInteract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractableEventDelegate_OnInteractEnd);

class UDefaultGameInstance;

UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class KARI_API UInteractableComponent : public UTableComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void OnTableIDChanged() override;

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void BroadCastOnInteract();

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void BroadCastOnInteractEnd();

	//Behaviour of interactable
	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FInteractableEventDelegate_OnInteract OnInteract;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FInteractableEventDelegate_OnInteractEnd OnInteractEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FString CharacterName = "Unnamed";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FString Action = "Interact with ";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FVector PositionOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Interactable")
	bool HasInteracted = false;

private:
	// The actual interaction logic, implemented in bp
	UFUNCTION()
	void Interact();

	UDefaultGameInstance* GameInstance = nullptr;
};
