#include "DefaultPlayer.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"

#include "Kari/General/DefaultGameInstance.h"
#include "Camera/CameraComponent.h"
#include "Characters/NPCs/DefaultNPC.h"

#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Systems/Components/Stats/DefaultStatComponent.h"
#include "NumericLimits.h"
#include "Systems/Audio/AudioManager.h"
#include "UI/HUD/HUDWidget.h"
#include "UI/HUD/ActionLog/ActionLogWidget.h"
#include "UI/HUD/Interact/HUDInteractWidget.h"

// Why is this used?
static int32 MovementStatEffect = -1;

// Sets default values
ADefaultPlayer::ADefaultPlayer()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	CameraSystem = CreateDefaultSubobject<UDefaultCameraSystemComponent>(TEXT("CameraSystem"));
	InteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));
	PlayerHUD = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerHUD"));
	PlayerActionHUDLog = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerActionHUDLog"));

	CameraSystem->bEnableCameraLag = true;
	CameraSystem->bEnableCameraRotationLag = true;
	CameraSystem->bUsePawnControlRotation = false;
	CameraSystem->bDoCollisionTest = false;

	CameraSystem->ProbeSize = 12.0f;

	CameraSystem->SetWorldRotation(FRotator(-40.0f, 0.0f, 0.0f));
	
	GetCharacterMovement()->GravityScale = 1.5f;

	GetCharacterMovement()->MaxAcceleration = 1000.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;

	GetCharacterMovement()->MaxWalkSpeed = 700.f;
	GetCharacterMovement()->JumpZVelocity = 420.f;
	GetCharacterMovement()->AirControl = 0.9f;
	GetCharacterMovement()->FallingLateralFriction = 0.f;

	PlayerHUD->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	PlayerActionHUDLog->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CameraSystem->GetCamera()->SetFieldOfView(75.f);
	CameraSystem->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	InteractionRange->SetupAttachment(RootComponent);
	InteractionRange->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	InteractionRange->InitSphereRadius(200.f);
	InteractionRange->OnComponentBeginOverlap.AddDynamic(this, &ADefaultPlayer::OnBeginOverlap);
	InteractionRange->OnComponentEndOverlap.AddDynamic(this, &ADefaultPlayer::OnEndOverlap);

	Blessing = CreateDefaultSubobject<UDefaultStatComponent>(TEXT("Blessing"));
}

// Called when the game starts or when spawned
void ADefaultPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Allows play from here
	SetActorRotation(FRotator::ZeroRotator);
	InitialRotation = GetMesh()->GetComponentRotation();

	// Initializes HUD
	PlayerHUD->SetWidget(GameInstance->GetHUD());
	PlayerActionHUDLog->SetWidget(GameInstance->GetHUDLog());
}

void ADefaultPlayer::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
}

// Called every frame
void ADefaultPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAudio();

	// Update player model rotation
	if (LastInputMovementDirection.Size() > 0.f)
	{
		FRotator NewRotator = LastInputMovementDirection.Rotation() + InitialRotation;
		FRotator WorldRotator = GetActorRotation() + GetMesh()->GetRelativeRotation();

		FRotator DiffRotator = NewRotator - WorldRotator;
		DiffRotator.Normalize();

		DiffRotator.Yaw = FMath::Clamp(DiffRotator.Yaw, -100.f, 100.f);
		DiffRotator *= 10.f * GetWorld()->GetDeltaSeconds();

		GetMesh()->AddWorldRotation(DiffRotator);
	}

	// Update interaction HUD
	if (!IsInputEnabled())
	{
		GameInstance->GetHUDInteract()->HidePopup();
		return;
	}

	AActor* ClosestInteractable = GetClosestInteractable();
	if (ClosestInteractable && !ClosestInteractable->IsPendingKill() && GameInstance->GetHUDInteract()->VisibilityAllowed)
	{
		UInteractableComponent* Interactable = Cast<UInteractableComponent>(ClosestInteractable->GetComponentByClass(UInteractableComponent::StaticClass()));
		ensure(Interactable);

		if(!GameInstance->GetHUDLog()->IsShowing())
			GameInstance->GetHUDInteract()->ShowPopup(BUTTON_TYPE::E_CONFIRM, Interactable->Action + Interactable->CharacterName);

		// Set interactable popup menu position
		FVector2D ScreenPosition;
		FVector Position;

		Position = ClosestInteractable->GetActorLocation();
		Position += Interactable->PositionOffset;

		GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen
		(
			Position,
			ScreenPosition,
			true
		);

		GameInstance->GetHUDInteract()->SetPosition(ScreenPosition);
		GameInstance->GetHUDInteract()->ToggleRequirements(ClosestInteractable);
	}
	else
	{
		GameInstance->GetHUDInteract()->HidePopup();
	}
}

// Called to bind functionality to input
void ADefaultPlayer::SetupPlayerInputComponent(UInputComponent* Component)
{
	Super::SetupPlayerInputComponent(Component);

	Component->BindAction("Confirm", IE_Pressed, this, &ADefaultPlayer::Confirm);

	Component->BindAxis("MoveX", this, &ADefaultPlayer::MoveX);
	Component->BindAxis("MoveY", this, &ADefaultPlayer::MoveY);
}

bool ADefaultPlayer::IsInteracting() const
{
	return InteractionCharacterID >= 0;
}

bool ADefaultPlayer::SaveData(bool Traversal)
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WriteData(ToBinary, Traversal);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result;
	if (Traversal)
	{
		result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Persistence\\PlayerSavedData.sav"));
	} else
	{
		result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\PlayerSavedData.sav"));
	}

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}
void ADefaultPlayer::WriteData(FArchive &Ar, bool Traversal)
{
	if (!Traversal)
	{
		// Save location data
		FVector Location = GetActorLocation();

		float z = Location.Z + 5.f;
		Ar << Location.X;
		Ar << Location.Y;
		Ar << z;
	}

	// Save blueprint data
	TArray<uint8> ObjectData;
	FMemoryWriter MemoryWriter(ObjectData, true); 

	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, false); 
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;
	Cast<AActor>(this)->Serialize(Archive);

	Ar << ObjectData;
}

bool ADefaultPlayer::LoadData(bool Traversal)
{
	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (Traversal)
	{
		if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Persistence\\PlayerSavedData.sav"))) return false;
	}
	else
	{
		if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\PlayerSavedData.sav"))) return false;
	}

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadData(FromBinary, Traversal);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void ADefaultPlayer::ReadData(FArchive &Ar, bool Traversal)
{
	FVector Location;
	if (!Traversal)
	{
		// Read location data
		Ar << Location.X;
		Ar << Location.Y;
		Ar << Location.Z;
	}

	// Read blueprint data
	TArray<uint8> ObjectData;
	Ar << ObjectData;

	FMemoryReader MemoryReader(ObjectData, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, false); 
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Apply blueprint data
	Cast<AActor>(this)->Serialize(Archive);
	this->Modify();

	if (!Traversal)
	{
		// Apply Location data
		SetActorLocation(Location, false);
		GetCameraSystem()->ResetArmLocation();
	}
}

void ADefaultPlayer::Respawn_Implementation()
{
	OnRespawn.Broadcast();
}

void ADefaultPlayer::SetInteractionCharacterID(int32 ID, bool IsTrader)
{ 
	InteractionCharacterID = ID; 
	if (ID == -INT_MAX)
		bIsInteractionCharacterTrader = false;
	else
		bIsInteractionCharacterTrader = IsTrader;
}

void ADefaultPlayer::OnInventoryChange_Implementation()
{

}

void ADefaultPlayer::MoveX(float Value)
{
	if (Controller && Value != 0.f)
	{
		AddMovementInput(FVector::RightVector, Value);
	}
	LastInputMovementDirection.Y = Value; // Yes, this needs to be Y. Ask Unreal why!
}

void ADefaultPlayer::MoveY(float Value)
{
	if (Controller && Value != 0.f)
	{
		AddMovementInput(FVector::ForwardVector, Value);
	}
	LastInputMovementDirection.X = Value; // Yes, this needs to be X. Ask Unreal why!
}

void ADefaultPlayer::UpdateAudio()
{
	// Change music based on velocity
	FVector GroundVelocity = GetVelocity();
	GroundVelocity.Z = 0;

	UPlaylist* Playlist = GameInstance->AudioManager->GetBGM();

	if (Playlist)
	{
		UAudio* MelodyLayer = Playlist->GetAudioLayer(0);
		UAudio* BeatLayer = Playlist->GetAudioLayer(1);

		if (BeatLayer)
		{
			// Maintain sync by never setting volume to 0
			float Volume = 0.01f;

			if (LastInputMovementDirection.Size() > 0.f)
			{
				Volume += 0.5f;
			}

			if (GroundVelocity.Size() > 50.f)
			{
				Volume += 0.5f;
			}

			const float MelodyVolume = MelodyLayer ? MelodyLayer->GetVolume() : 1.f;
			Volume = FMath::Clamp(Volume, 0.f, 1.f) * MelodyVolume;

			if (BeatLayer->GetLastVolume() != Volume)
			{
				BeatLayer->FadeVolume(BeatLayer->GetVolume(), Volume, 1.f);
			}
		}
	}
}

void ADefaultPlayer::Confirm()
{
	AActor* Interactable = GetClosestInteractable();

	if (!Interactable)
		return;

	UInteractableComponent* Component = Cast<UInteractableComponent>(Interactable->GetComponentByClass(UInteractableComponent::StaticClass()));
	ensure(Component);
	Component->OnInteract.Broadcast();

	OnInteract(Interactable);
}

AActor* ADefaultPlayer::GetClosestInteractable() const
{
	if (Interactables.Num() == 0)
		return nullptr;

	AActor* Interactable = nullptr;
	float MinDistance = TNumericLimits<float>::Max();

	for (auto& InteractableActor : Interactables)
	{
		AActor* Actor = InteractableActor.Key;

		// Calculate the angle between the player forward direction and the iteractable.
		FVector InteractableDirection = Actor->GetActorLocation() - GetMesh()->GetComponentLocation();
		InteractableDirection.Normalize();

		float InteractableAngle = FVector::DotProduct(GetMesh()->GetForwardVector(), InteractableDirection);
		InteractableAngle = FMath::RadiansToDegrees(FMath::Acos(InteractableAngle));

		if (InteractableAngle < InteractionConeAngle)
		{
			float Distance = FVector::Distance(Actor->GetActorLocation(), GetActorLocation());
			if (Distance < MinDistance)
			{
				Interactable = Actor;
				MinDistance = Distance;
			}
		}
	}

	return Interactable;
}

void ADefaultPlayer::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OverlappedComponent == InteractionRange && OtherActor->FindComponentByClass(UInteractableComponent::StaticClass()))
	{
		unsigned int* InteractableActorCount = Interactables.Find(OtherActor);
		if (InteractableActorCount == nullptr)
			Interactables.Add(OtherActor, 1);
		else
			(*InteractableActorCount)++;
	}
}

UActionLogWidget* ADefaultPlayer::GetHUDLog() const
{
	return GameInstance ? GameInstance->GetHUDLog() : nullptr;
}

bool ADefaultPlayer::IsInputEnabled() const
{
	return InputEnabled();
}

void ADefaultPlayer::OnEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (OverlappedComponent == InteractionRange && OtherActor->FindComponentByClass(UInteractableComponent::StaticClass()))
	{
		unsigned int* InteractableActorCount = Interactables.Find(OtherActor);

		if (!InteractableActorCount)
			return;
		
		(*InteractableActorCount)--;

		if ((*InteractableActorCount) == 0)
			Interactables.Remove(OtherActor);
	}
}

