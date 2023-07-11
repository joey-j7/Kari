#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "PersistenceLibrary.generated.h"

UCLASS()
class KARI_API UPersistenceLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UPersistenceLibrary() = default;

	UFUNCTION(BlueprintCallable, Category = "Persistence", meta = (WorldContext = "WorldContextObject"))
	static float GetMistRadiusMultiplier(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Persistence", meta = (WorldContext = "WorldContextObject"))
	static float GetMistDamageMultiplier(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Persistence", meta = (WorldContext = "WorldContextObject"))
	static void SetMistRadiusMultiplier(UObject* WorldContextObject, float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Persistence", meta = (WorldContext = "WorldContextObject"))
	static void SetMistDamageMultiplier(UObject* WorldContextObject, float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Persistence", meta = (WorldContext = "WorldContextObject"))
	static bool GetGhostPassAccess(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Persistence", meta = (WorldContext = "WorldContextObject"))
	static void SetGhostPassAccess(UObject* WorldContextObject, bool access);

	UFUNCTION(BlueprintCallable, Category = "Persistence", meta = (WorldContext = "WorldContextObject"))
	static bool GetSunBeamReflection(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Persistence", meta = (WorldContext = "WorldContextObject"))
	static void SetSunBeamReflection(UObject* WorldContextObject, bool reflect);
};