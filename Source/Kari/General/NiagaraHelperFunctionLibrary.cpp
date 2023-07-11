#include "NiagaraHelperFunctionLibrary.h"

bool UNiagaraHelperFunctionLibrary::EnsureNiagaraInitialized(UNiagaraComponent* ParticleComponent)
{
	if (!ParticleComponent || !ParticleComponent->GetSystemInstance())
		return false;
	return !ParticleComponent->GetSystemInstance()->IsPendingSpawn();
}