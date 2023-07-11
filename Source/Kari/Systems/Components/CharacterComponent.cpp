#include "CharacterComponent.h"
#include "StoryComponent.h"
#include "Crafting/DefaultCraftingComponent.h"

UCharacterComponent::UCharacterComponent() : UTableComponent("characters", "name")
{

}

void UCharacterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterComponent::BeginDestroy()
{
	Super::BeginDestroy();
}

void UCharacterComponent::OnTableIDChanged()
{
	if (StoryComponent)
	{
		StoryComponent->DropdownDisabled = ID == UINT_MAX;
		StoryComponent->SetTableID(UINT_MAX);
		StoryComponent->SetLinkedID(ID);
	}
	
	if (TradeComponent)
	{
		TradeComponent->DropdownDisabled = ID == UINT_MAX;
		TradeComponent->SetTableID(UINT_MAX);
		TradeComponent->SetLinkedID(ID);
	}

	Super::OnTableIDChanged();
}
