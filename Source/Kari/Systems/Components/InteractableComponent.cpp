#include "InteractableComponent.h"

#include <string>

#include "General/DefaultGameInstance.h"
#include "Systems/Quests/QuestLog.h"
#include "Characters/Playables/DefaultPlayer.h"

UInteractableComponent::UInteractableComponent() :
	UTableComponent("nonPlayableCharacters", "name")
{
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UDefaultGameInstance>(GetOwner()->GetGameInstance());
	if (!OnInteract.Contains(this, "Interact"))
		OnInteract.AddDynamic(this, &UInteractableComponent::Interact);
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnInteract.Clear();
	OnInteractEnd.Clear();
}
// This is needed for some reason or it will crash in the Table Plugin.
void UInteractableComponent::OnTableIDChanged()
{
	Super::OnTableIDChanged();
}

void UInteractableComponent::BroadCastOnInteract()
{
	OnInteract.Broadcast();
}

void UInteractableComponent::BroadCastOnInteractEnd()
{
	OnInteractEnd.Broadcast();
}

void UInteractableComponent::Interact()
{
	if(ID == UINT_MAX)
		return;

	// Get the instance and call the log that we interacted with something.
	GameInstance->QuestLog->OnInteractWithTask(this);
}
