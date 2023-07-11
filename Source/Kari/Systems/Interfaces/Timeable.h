#pragma once

#include "CoreMinimal.h"

#include "Interface.h"
#include "Systems/Time/Period.h"

#include "Timeable.generated.h"

class UDefaultGameInstance;

UINTERFACE(Blueprintable)
class KARI_API UTimeable : public UInterface
{
	GENERATED_BODY()
};

class KARI_API ITimeable
{
	GENERATED_BODY()
	friend class UTimeManager;

protected:
	virtual void OnPeriod(EPeriod Period, UObject* Caller);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnNight();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnMorning();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnAfternoon();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnEvening();

	UDefaultGameInstance* GameInstance = nullptr;
};