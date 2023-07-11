#include "CameraShotTrigger.h"
#include <Engine/World.h>

#include "General/DefaultGameInstance.h"
#include "UI/HUD/ActionLog/ActionLogWidget.h"

ACameraShotTrigger::ACameraShotTrigger()
{
	OnActorBeginOverlap.AddDynamic(this, &ACameraShotTrigger::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ACameraShotTrigger::OnOverlapEnd);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera Destination");
	CameraComponent->SetupAttachment(RootComponent);
}

void ACameraShotTrigger::BeginPlay()
{
	Super::BeginPlay();
	// Disable Camera Component here?
}

void ACameraShotTrigger::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Player)
		return;

	float Radius = Cast<USphereComponent>(GetCollisionComponent())->GetScaledSphereRadius();
	Offset = FMath::Min(Radius, Offset);

	float Position = 1.0f - FVector::Distance(GetActorLocation(), Player->GetActorLocation()) / Radius;
	Position = FMath::Clamp(Position, 0.f, 1.f);

	const UDefaultCameraSystemComponent* CameraSystem = Player->GetCameraSystem();

	// Do this to determine position at the end of spring arm
	CameraSystem->GetCamera()->SetRelativeLocation(InitialLocation);

	const FVector CurrentLocation = FMath::Lerp(
		CameraSystem->GetCamera()->GetComponentLocation(),
		CameraComponent->GetComponentLocation(),
		Position
	);

	const FRotator CurrentRotation = FMath::Lerp(CameraSystem->GetComponentRotation() + InitialRotation, CameraComponent->GetComponentRotation(), Position);

	CameraSystem->GetCamera()->SetWorldLocation(CurrentLocation);
	CameraSystem->GetCamera()->SetWorldRotation(CurrentRotation);
}

void ACameraShotTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (Player)
		return;

	Player = Cast<ADefaultPlayer>(OtherActor);

	if (!Player)
		return;

	InitialLocation = Player->GetCameraSystem()->GetCamera()->GetRelativeLocation();
	InitialRotation = Player->GetCameraSystem()->GetCamera()->GetRelativeRotation();

	const auto HUDLog = Player->GetHUDLog();
	if(HUDLog)
		HUDLog->IsInCameraShot(true);
	
}

void ACameraShotTrigger::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if (Cast<ADefaultPlayer>(OtherActor))
	{
		if (Player)
		{
			Player->GetCameraSystem()->GetCamera()->SetRelativeLocationAndRotation(InitialLocation, InitialRotation);
			const auto HUDLog = Player->GetHUDLog();
			if(HUDLog)
				HUDLog->IsInCameraShot(false);

			Player = nullptr;
		}
	}
}