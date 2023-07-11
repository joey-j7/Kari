#include "DialogueInteractable.h"

#include "Characters/Playables/DefaultPlayer.h"

#include "TableDatabase.h"
#include "Systems/Components/CharacterComponent.h"
#include "Systems/Components/StoryComponent.h"
#include "Systems/Revisioning/Revisioning.h"
#include "Systems/Quests/QuestLog.h"

#include "General/DefaultGameInstance.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ConstructorHelpers.h"

ADialogueInteractable::ADialogueInteractable()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	MeshComponent->SetStaticMesh(Cube.Object);

	CharacterComponent = CreateDefaultSubobject<UCharacterComponent>(TEXT("CharacterComponent"));
	StoryComponent = CreateDefaultSubobject<UStoryComponent>(TEXT("StoryComponent"));
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(FName("InteractableComponent"));

	CharacterComponent->StoryComponent = StoryComponent;
	StoryComponent->CharacterComponent = CharacterComponent;
}

void ADialogueInteractable::BeginPlay()
{
	Super::BeginPlay();

	InteractableComponent->OnInteract.AddDynamic(this, &ADialogueInteractable::OnInteract_Implementation);	
	InteractableComponent->OnInteractEnd.AddDynamic(this, &ADialogueInteractable::OnInteractEnd_Implementation);	

	// Set character name on interaction UI
	const FTableField* Field = TableDatabase::Get().GetField("characters", "name", CharacterComponent->GetTableID());
	std::string CharacterName;
	CharacterName.resize(Field->Size);
	memcpy((void*)CharacterName.c_str(), Field->Data.Get(), Field->Size);
	InteractableComponent->CharacterName = FString(CharacterName.c_str());

	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());
	GameInstance->Revisioning->AddDialogueInteractable(*this);
}

void ADialogueInteractable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(this->GetWorld(), 0));

	if (IsInteracting)
	{
		FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(
			Player->GetActorLocation(),
			GetActorLocation()
		);

		Rotation.Pitch = 0.f;
		Rotation.Roll = 0.f;

		FRotator FullRotation = Player->GetActorRotation() + Player->GetMesh()->GetComponentRotation();
		FRotator Diff = Rotation + Player->GetInitialRotation() - FullRotation;
		Diff.Normalize();

		Diff.Yaw = FMath::Clamp(Diff.Yaw, -100.f, 100.f);

		Rotation = FullRotation + Diff * 10.f * DeltaSeconds;

		Player->GetMesh()->SetWorldRotation(Rotation);
	}
}

void ADialogueInteractable::OnInteract_Implementation()
{
	if (!StoryComponent)
		return;

	if (!StoryComponent->ShowDialogue())
		return;

	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(this->GetWorld(), 0));
	IsInteracting = true;
}

void ADialogueInteractable::OnInteractEnd_Implementation()
{
	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(this->GetWorld(), 0));

	if (GameInstance->QuestLog->NPCQuestFlag)
	{
		// Quest music
		// FAkAudioDevice::Get()->Play(TEXT("OnQuest"), GetOwner());
		GameInstance->QuestLog->NPCQuestFlag = false;
	}

	IsInteracting = false;
}
