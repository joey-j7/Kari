#include "RestorePoint.h"
#include "Systems/Components/Stats/DefaultStatComponent.h"

#include "Kismet/GameplayStatics.h"

ARestorePoint::ARestorePoint()
{
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(FName("InteractableComponent"));
}

void ARestorePoint::BeginPlay()
{
	Super::BeginPlay();

	InteractableComponent->OnInteract.AddDynamic(this, &ARestorePoint::OnInteract_Implementation);	
	InteractableComponent->CharacterName = FString("Restore Point");
}

void ARestorePoint::OnInteract_Implementation()
{
	auto ToReset = static_cast<UDefaultStatComponent*>(StatReference.GetComponent(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)));

#if UE_BUILD_DEVELOPMENT
	if (!ToReset)
	{
		UE_LOG(LogTemp, Error, TEXT("Stat not found"));
		return;
	}
#endif
	ToReset->Reset();
}