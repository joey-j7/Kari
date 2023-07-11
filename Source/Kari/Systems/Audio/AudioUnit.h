#pragma once

#include "CoreMinimal.h"
#include "AudioUnit.generated.h"

class UAudioManager;

UCLASS()
class UAudioUnit : public UObject
{
	GENERATED_BODY()

public:
	UAudioUnit() {};

	void SetAudioManager(UAudioManager* M) { Manager = M; };

	UFUNCTION(BlueprintCallable)
	float GetVolume() const;

	UFUNCTION(BlueprintCallable)
	void SetVolume(float V);

	UFUNCTION(BlueprintCallable)
	void SetModifier(float V);

	UFUNCTION(BlueprintCallable)
	void SetUserSetting(float V);

	UFUNCTION(BlueprintCallable)
	void SetParent(UAudioUnit* P);

	UFUNCTION(BlueprintCallable)
	void SaveUserSetting(FString Name);

	bool IsDirty() const { return Dirty; }
	void Clean() { Dirty = false; };

protected:
	UAudioManager* Manager = nullptr;
	UAudioUnit* Parent = nullptr;

	bool Dirty = false;

	UPROPERTY(BlueprintReadOnly)
	float Volume = 1.f;

	UPROPERTY(BlueprintReadOnly)
	float Modifier = 1.f;

	UPROPERTY(BlueprintReadOnly)
	float UserSetting = 1.f;
};