#include "DefaultCameraSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DefaultGameInstance.h"

UDefaultCameraSystemComponent::UDefaultCameraSystemComponent()
{
	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(this);

#if PLATFORM_ANDROID || PLATFORM_IOS
	TargetArmLength = MobileArmLength;
#else
	TargetArmLength = 1200.f;
#endif
}

void UDefaultCameraSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(true);

	DefaultArmLength = TargetArmLength;
}

void UDefaultCameraSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update camera zoom
	TargetArmLength = FMath::Lerp(
		TargetArmLength,
		Zoomed ? FocussedArmLength : DefaultArmLength,
		ZoomSpeed * 0.001f
	);
}

void UDefaultCameraSystemComponent::ResetArmLocation()
{
	UpdateDesiredArmLocation(false, false, false, 0 /* not used inside */);
}

void UDefaultCameraSystemComponent::Zoom()
{
	Zoomed = true;
}

void UDefaultCameraSystemComponent::Unzoom()
{
	Zoomed = false;
}