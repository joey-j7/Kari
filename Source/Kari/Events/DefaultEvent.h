#pragma once

#include "CoreMinimal.h"
#include "General/DefaultGameInstance.h"

#include "DefaultEvent.generated.h"

UCLASS(Blueprintable)
class KARI_API UDefaultEvent : public UObject
{
	GENERATED_BODY()

public:
	UDefaultEvent() { }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Events)
	void OnEnter();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Events)
	void OnExit();

	UFUNCTION(BlueprintCallable, Category = Events)
	UDefaultGameInstance* GetGameInstance() const;
};