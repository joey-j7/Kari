#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"

#include "General/DefaultGameInstance.h"

#include "DefaultTrigger.generated.h"

UCLASS(Blueprintable)
class KARI_API ADefaultTrigger : public ATriggerBox
{
	GENERATED_BODY()

public:
	ADefaultTrigger();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Events)
	void OnEnter();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Events)
	void OnExit();

	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);
	
	UFUNCTION(BlueprintCallable, Category = Events)
	UDefaultGameInstance* GetGameInstance() const;
};