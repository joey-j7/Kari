#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "NiagaraHelperFunctionLibrary.generated.h"

UCLASS()
class KARI_API UNiagaraHelperFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UNiagaraHelperFunctionLibrary() = default;

	UFUNCTION(BlueprintCallable, Category = "Item", meta = (DisplayName = "Ensure Niagara Initialized"))
	static bool EnsureNiagaraInitialized(UNiagaraComponent* ParticleComponent);
};