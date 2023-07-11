

#pragma once

#include "CoreMinimal.h"
#include "Characters/DefaultCharacter.h"
#include "Systems/Effects/StatEffect.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

#include "General/DefaultCameraSystemComponent.h"

#include "DefaultPlayer.generated.h"


class ADefaultPlayer;
class USpringArmComponent;
class UCameraComponent;
// class UCraftingComponent;
class UDefaultStatComponent;
class UActionLogWidget;

/**
 * Event dispatcher to notify that the ui has taken over.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIOverlayDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDefaultPlayerDelegate_OnRespawn);

/**
 * @brief class Player character
 * This will have basic information about
 * the characters in the game.
 * This is data that they share across classes.
 */
UCLASS()
class KARI_API ADefaultPlayer : public ADefaultCharacter
{
	friend class UDebugWidget;
	friend class UInventory;

	GENERATED_BODY()
		// Line skip 1
		// Line skip 2
public:
	// Sets default values for this character's properties
	ADefaultPlayer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	bool IsInteracting() const;

	UFUNCTION(BlueprintCallable)
	AActor* GetClosestInteractable() const;

	UFUNCTION(BlueprintCallable)
	const FRotator& GetInitialRotation() const { return InitialRotation; };

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "PlayerCharacter")
	void OnInteract(AActor* Interactable);

	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	UDefaultCameraSystemComponent* GetCameraSystem() const { return CameraSystem; }

	UActionLogWidget* GetHUDLog() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsInputEnabled() const;

	UFUNCTION()
	void OnEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UDefaultStatComponent* GetBlessing() const { return Blessing; }
	USphereComponent* GetInteractionRange() const { return InteractionRange; }

	// Save/Load
	bool SaveData(bool Traversal);
	void WriteData(FArchive &Ar, bool Traversal);
	bool LoadData(bool Traversal);
	void ReadData(FArchive &Ar, bool Traversal);

	UFUNCTION(Category = "Game begin", BlueprintImplementableEvent, BlueprintCallable)
	void GameBegin();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerCharacter")
	void Respawn();

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	int32 GetInteractionCharacterID() const { return InteractionCharacterID; };

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	bool IsInteractionCharacterTrader() const { return bIsInteractionCharacterTrader; };

	void SetInteractionCharacterID(int32 ID, bool IsTrader = false);

	UPROPERTY(BlueprintAssignable, Category = "PlayerCharacter")
	FDefaultPlayerDelegate_OnRespawn OnRespawn;

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerCharacter")
	void OnInventoryChange();
	
	// Movement translate
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	void MoveX(float fValue);

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	void MoveY(float fValue);

	void UpdateAudio();

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	void Confirm();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called when the game ends, on level traversal, or when deleted
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
	TMap<AActor*, unsigned int> Interactables;

	// Camera system, which is a spring arm
	UPROPERTY(Category = PlayerCharacter, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UDefaultCameraSystemComponent* CameraSystem = nullptr;

	UPROPERTY(Category = PlayerCharacter, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float InteractionConeAngle = 70.0f;

	UPROPERTY(Category = PlayerCharacter, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USphereComponent* InteractionRange = nullptr;

	UPROPERTY(Category = PlayerCharacter, VisibleAnywhere, BlueprintReadOnly)
	UDefaultStatComponent* Blessing = nullptr;
	
	UPROPERTY(Category = PlayerCharacter, VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* PlayerHUD = nullptr;

	UPROPERTY(Category = PlayerCharacter, VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* PlayerActionHUDLog = nullptr;

	int32 InteractionCharacterID = -INT_MAX;
	bool bIsInteractionCharacterTrader = false;

	FRotator InitialRotation;

	UPROPERTY(Category = PlayerCharacter, VisibleAnywhere, BlueprintReadWrite)
	FVector LastInputMovementDirection;
};