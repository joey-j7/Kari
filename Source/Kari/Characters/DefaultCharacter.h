

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "DefaultCharacter.generated.h"

/**
 * Event dispatcher to notify that the ui has taken over.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDefaultCharacterDelegate_OnVisibilityChange);

class UDefaultGameInstance;

UCLASS()
class KARI_API ADefaultCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADefaultCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void ShowOffScreen();

	UFUNCTION(BlueprintCallable)
	void HideOffScreen();

protected:
	UPROPERTY(BlueprintAssignable, Category = "DefaultCharacter")
	FDefaultCharacterDelegate_OnVisibilityChange OnVisibilityChange;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	UDefaultGameInstance* GameInstance = nullptr;

	bool FirstFrame = true;
	bool ShouldHide = false;
};
