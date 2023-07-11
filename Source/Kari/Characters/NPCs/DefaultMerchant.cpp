#include "DefaultMerchant.h"

#include "Systems/Components/Crafting/TradeComponent.h"
#include "UI/Dialogues/DialogueWidget.h"
#include "Systems/Components/StoryComponent.h"
#include "Systems/Revisioning/Revisioning.h"
#include "General/DefaultGameInstance.h"

ADefaultMerchant::ADefaultMerchant()
{
	TradeComponent = CreateDefaultSubobject<UTradeComponent>(TEXT("TradeComponent"));

	TradeComponent->CharacterComponent = CharacterComponent;
	CharacterComponent->TradeComponent = TradeComponent;
}

void ADefaultMerchant::BeginPlay()
{
	Super::BeginPlay();

	if(GameInstance)
		GameInstance->Revisioning->AddMerchant(*this);
}

void ADefaultMerchant::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Remove the object from the revisioning Merchant list
	if (GameInstance)
		GameInstance->Revisioning->RemoveMerchant(*this);
}

void ADefaultMerchant::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

UDefaultCraftingComponent* ADefaultMerchant::GetTradeComponent() const
{
	return TradeComponent;
}


void ADefaultMerchant::OnInteract_Implementation()
{
	Super::OnInteract_Implementation();
}

void ADefaultMerchant::OnInteractEnd_Implementation()
{
	Super::OnInteractEnd_Implementation();
}