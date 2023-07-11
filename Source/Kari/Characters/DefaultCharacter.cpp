

#include "DefaultCharacter.h"
#include "Components/CapsuleComponent.h"

#include "General/DefaultGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADefaultCharacter::ADefaultCharacter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
	FirstFrame = true;
}

// Called every frame
void ADefaultCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsHidden() != ShouldHide)
	{
		if (GetLastRenderTime() > 0.f)
		{
			FVector2D ScreenPosition;

			UGameplayStatics::ProjectWorldToScreen(
				UGameplayStatics::GetPlayerController(GetWorld(), 0),
				GetActorLocation(),
				ScreenPosition,
				true
			);

			const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			const FVector2D Offset = FVector2D(50.f, 50.f);

			if (ScreenPosition >= FVector2D::ZeroVector - Offset && ScreenPosition <= ViewportSize + Offset)
				return;
		}

		SetActorHiddenInGame(ShouldHide);
		SetActorEnableCollision(!ShouldHide);

		OnVisibilityChange.Broadcast();
	}

	FirstFrame = false;
}

void ADefaultCharacter::ShowOffScreen()
{
	ShouldHide = false;
}

void ADefaultCharacter::HideOffScreen()
{
	ShouldHide = true;
}

