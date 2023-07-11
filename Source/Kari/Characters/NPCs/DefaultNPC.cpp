#include "DefaultNPC.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

#include "Characters/Playables/DefaultPlayer.h"

#include "TableDatabase.h"
#include "Systems/Components/CharacterComponent.h"
#include "Systems/Components/StoryComponent.h"
#include "Systems/Revisioning/Revisioning.h"
#include "Systems/Quests/QuestLog.h"

#include "General/DefaultGameInstance.h"

ADefaultNPC::ADefaultNPC()
{
	CharacterComponent = CreateDefaultSubobject<UCharacterComponent>(TEXT("CharacterComponent"));
	StoryComponent = CreateDefaultSubobject<UStoryComponent>(TEXT("StoryComponent"));
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(FName("InteractableComponent"));

	CharacterComponent->StoryComponent = StoryComponent;
	StoryComponent->CharacterComponent = CharacterComponent;
}

void ADefaultNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Remove the object from the revisioning NPC list
	if (GameInstance)
		GameInstance->Revisioning->RemoveNPC(*this);
}

void ADefaultNPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ADefaultNPC::BeginPlay()
{
	Super::BeginPlay();

	InteractableComponent->OnInteract.AddDynamic(this, &ADefaultNPC::OnInteract_Implementation);	
	InteractableComponent->OnInteractEnd.AddDynamic(this, &ADefaultNPC::OnInteractEnd_Implementation);	
	#if UE_BUILD_DEVELOPMENT        
	if(!GameInstance)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Game instance is not defined!"));
        return;
    }
#endif
	// Set character name on interaction UI
	const FTableField* Field = TableDatabase::Get().GetField("characters", "name", CharacterComponent->GetTableID());
	std::string CharacterName;
	CharacterName.resize(Field->Size);
	memcpy((void*)CharacterName.c_str(), Field->Data.Get(), Field->Size);
	InteractableComponent->CharacterName = FString(CharacterName.c_str());
	InteractableComponent->Action = "Talk to ";

	GameInstance->Revisioning->AddNPC(*this);
}

void ADefaultNPC::OnInteract_Implementation()
{
	if (!StoryComponent)
		return;

	if (!StoryComponent->ShowDialogue())
		return;

	IsInteracting = true;

	if (RotateToPlayer)
		GetWorld()->GetTimerManager().SetTimer(RotationTimerHandle, this, &ADefaultNPC::InteractRotateImpl, 1.0f / 60.0f, true, 0.f);
}

void ADefaultNPC::OnInteractEnd_Implementation()
{
	if (GameInstance->QuestLog->NPCQuestFlag)
	{
		// Quest music
		// Play(TEXT("OnQuest"), GetOwner());
		GameInstance->QuestLog->NPCQuestFlag = false;
	}

	IsInteracting = false;

	if (RotateToPlayer)
		GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
}

void ADefaultNPC::InteractRotateImpl()
{
	if (!RotateToPlayer)
		return;

	InteractRotateImpl(GetWorld()->GetTimerManager().GetTimerRate(RotationTimerHandle));
}
void ADefaultNPC::InteractRotateImpl(float TimerRate)
{
	if (!RotateToPlayer)
		return;

	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(this->GetWorld(), 0));
	
	// Rotate NPC
	FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(
		GetActorLocation(),
		Player->GetActorLocation()
	);

	Rotation.Pitch = 0.f;
	Rotation.Roll = 0.f;

	SetActorRotation(FMath::RInterpTo(GetActorRotation(), Rotation, TimerRate, RotationSpeed));

	// Rotate Kari
	Rotation = UKismetMathLibrary::FindLookAtRotation(
		Player->GetActorLocation(),
		GetActorLocation()
	);

	Rotation.Pitch = 0.f;
	Rotation.Roll = 0.f;

	FRotator FullRotation = Player->GetActorRotation() + Player->GetMesh()->GetComponentRotation();
	FRotator RotateTo = Rotation + Player->GetInitialRotation();
	Player->GetMesh()->SetWorldRotation(FMath::RInterpTo(FullRotation, RotateTo, TimerRate, RotationSpeed));
}