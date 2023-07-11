// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestLogWidget.h"


#include "TableDatabase.h"
#include "General/DefaultGameInstance.h"
#include "Systems/Quests/QuestLog.h"
#include "Systems/Audio/AudioManager.h"

void UQuestLogWidget::NativeConstruct()
{
	UNavigatableWidget::NativeConstruct();
	
	// Disable this button
	Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &UQuestLogWidget::OnInventory);
	Input->BindAction("Pause", EInputEvent::IE_Pressed, this, &UQuestLogWidget::OnPause);
	Input->BindAction("Exit", EInputEvent::IE_Pressed, this, &UQuestLogWidget::OnCancel);
	Input->BindAction("QuestLog", EInputEvent::IE_Pressed, this, &UQuestLogWidget::OnCancel);

	// Slowmotion effect
   	GameInstance->SetGameSpeed(0.33f);
	GameInstance->AudioManager->Play({ "Global/UI/Confirm" });
}

void UQuestLogWidget::NativeDestruct()
{
	UNavigatableWidget::NativeDestruct();

	// Revert Slowmotion effect
	GameInstance->SetGameSpeed(1.f);

	// Play whoosh sound TODO sound FX
	// GameInstance->AudioManager->Play({ "Global/UI/Inventory/Close" })->SetVolume(0.4f);
}

void UQuestLogWidget::OnCancel_Implementation()
{
	GameInstance->AudioManager->Play({ "Global/UI/Cancel" });
}

FString UQuestLogWidget::GetCharacterName()
{
	FString CharacterName = "";
	
	// Set character name on interaction UI
	if(const int32* ID =  QuestLog->FindCharacterID(CurrentQuest.ID))
	{
		const FTableField* Field = TableDatabase::Get().GetField("characters", "name", *ID);
		std::string Name;
		Name.resize(Field->Size);
		memcpy((void*)Name.c_str(), Field->Data.Get(), Field->Size);
		CharacterName = FString(Name.c_str());
	}

	return CharacterName;
}

FString UQuestLogWidget::GenerateHeader(const FQuest& Quest)
{
	const FString Char = GetCharacterName();
	return (Char.Len() > 0 ? " : " : "") + Quest.Title;
}

void UQuestLogWidget::OnInventory_Implementation()
{
	
}

