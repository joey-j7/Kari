#include "DialogueWidget.h"

#include <string>

#include "WidgetTree.h"
#include "CanvasPanelSlot.h"

#include "TableDatabase.h"
#include "TableBinary.h"

#include "Systems/Components/StoryComponent.h"
#include "Systems/Components/DialogueOptionEventComponent.h"
#include "Characters/Playables/DefaultPlayer.h"
#include "General/DefaultGameInstance.h"
#include "Systems/Audio/Playlist.h"
#include "Systems/Audio/AudioManager.h"

UDialogueWidget::UDialogueWidget(const FObjectInitializer& ObjectInitializer) : UNavigatableWidget(ObjectInitializer)
{
}

void UDialogueWidget::StopSFX()
{
	if (ScrollSFX)
		ScrollSFX->Stop();
}

void UDialogueWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!AnimateText)
		return;

	ScrollText(SkipText ? (SkipSpeed * InDeltaTime) : (ScrollSpeed * InDeltaTime));
}

void UDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());

	// Disable this button
	Input->BindAction("ToggleInventory", EInputEvent::IE_Pressed, this, &UDialogueWidget::OnInventory);
	Input->BindAction("QuestLog", EInputEvent::IE_Pressed, this, &UDialogueWidget::OnQuestLog);
	Input->BindAction("Pause", EInputEvent::IE_Pressed, this, &UDialogueWidget::OnPause);
}

void UDialogueWidget::SetDialogue(const FText& Text, const FString& Name, int32 CurrentID, int32 NextID)
{
	if (!GameInstance)
		GameInstance = Cast<UDefaultGameInstance>(GetGameInstance());

	FullText = Text;
	CharacterName = Name;

	AnimateText = true;

	ScrollSFX = GameInstance->AudioManager->Play({ "Global/UI/Dialogue/Scroll" }, true, 0, 0, 0.75f, 1.0f);
	ScrollSFX->SetVolume(0.3f);

	SkipText = false;
	OptionShown = false;
	LeaveOption = { UINT_MAX, UINT_MAX };
	
	TextCounter = 0;
	CurrentTextChar = 0;

	DialogID = CurrentID;
	NextDialogID = NextID;

	OnTextChanged(Text);
}

void UDialogueWidget::AddOption(const std::string& Text, std::pair<uint32, uint32> Option)
{
	const FString OptionText = UTF8_TO_TCHAR(Text.c_str());
	Options.Add(std::pair<FString, std::pair<uint32, uint32>>(OptionText, Option));
	OnOptionAdded(OptionText);

	// Set leave option if text has been found
	const auto Find = LeaveOptions.Find(OptionText);

	if (Find != INDEX_NONE)
	{
		LeaveOption = Option;
	}
}

void UDialogueWidget::OnNavChange_Implementation(bool Reset)
{
	Super::OnNavChange_Implementation(Reset);

	// Play select SFX
	SelectSFX = GameInstance->AudioManager->Play({ "Global/UI/Select" }, false);
}

void UDialogueWidget::OnCancel_Implementation()
{
	Super::OnCancel_Implementation();

	if(OptionShown && StoryComponent)
	{
		if (Options.Num() > 0)
		{
			// std::pair<uint32 ID, uint32 NextID>
			NextDialogID = Options[CurrentMenuOption].second.second;
		
			// If we have shown the option and the player presses the cancel button
			if(OptionShown && LeaveOption.first != UINT_MAX)
			{
				// set the next id to the leave option second value
				NextDialogID = LeaveOption.second;

				// Play the confirm sound for "on back"
				ConfirmSFX = GameInstance->AudioManager->Play({ "Global/UI/Confirm" }, false);

				// Check if there are events in leave option
				InitEvents(LeaveOption.first);

				// Advance the story
				StoryComponent->Advance();

				// Call all events
				for (int j = 0; j < Events.Num(); j++)
				{
					UDialogueOptionEventComponent::OnDialogueOptionEventDelegate.Broadcast(Events[j]);
				}
			}
		}
	}
}


void UDialogueWidget::ClearOptions_Implementation()
{
	// TODO maybe add on exit method for the the Events
	Events.Empty();
	
	Options.Empty();

	// MaxPosition.Y = 0;
}

void UDialogueWidget::ScrollText(float scrollSpeed)
{
	TextCounter += scrollSpeed;

	// For each character we scroll.
	int CharactersToScroll = (int)TextCounter - CurrentTextChar;
	for (int i = 0; i < CharactersToScroll; i++)
	{
		// Skip all html tags.
		bool MatchedTag = true;
		while (MatchedTag)
		{
			MatchedTag = false;
			MatchedTag = SkipHTMLTag(" ", "") ? true : MatchedTag; // Skip whitespace too.
			MatchedTag = SkipHTMLTag("<item>", "</>") ? true : MatchedTag;
			MatchedTag = SkipHTMLTag("<b>", "</>") ? true : MatchedTag;
		}

		CurrentTextChar++;
	}

	const FString Output = FullText.ToString().Left(CurrentTextChar).Append(ScrollEndingString);
	OnAnimate(FText::FromString(Output));

	if (TextCounter >= FullText.ToString().Len())
		OnAnimationFinish();
}

bool UDialogueWidget::SkipHTMLTag(const FString & Tag, const FString & EnclosingTag)
{
	bool Matched = false;

	// Match against tag.
	if (Tag.Len() > 0 && CurrentTextChar + Tag.Len() < FullText.ToString().Len())
	{
		if (FullText.ToString().Mid(CurrentTextChar, Tag.Len()).Compare(Tag, ESearchCase::IgnoreCase) == 0)
		{
			CurrentTextChar += Tag.Len();
			TextCounter += Tag.Len();
			ScrollEndingString.InsertAt(0, EnclosingTag);
			Matched = true;
		}
	}

	// Match against enclosing tag.
	if (EnclosingTag.Len() > 0 && CurrentTextChar + EnclosingTag.Len() < FullText.ToString().Len())
	{
		if (FullText.ToString().Mid(CurrentTextChar, EnclosingTag.Len()).Compare(EnclosingTag, ESearchCase::IgnoreCase) == 0)
		{
			CurrentTextChar += EnclosingTag.Len();
			TextCounter += EnclosingTag.Len();
			ScrollEndingString.RemoveFromStart(EnclosingTag);
			Matched = true;
		}
	}

	return Matched;
}

bool UDialogueWidget::Advance()
{
	if (AnimateText)
	{
		Skip();
		return false;
	}

	if (!OptionShown && StoryComponent)
	{
		if (StoryComponent->CheckNext())
		{
			OptionShown = true;
			return false;
		}
	}

	if (Options.Num() > 0)
	{
		// std::pair<uint32 ID, uint32 NextID>
		const auto Pair = Options[CurrentMenuOption].second;
		NextDialogID = Pair.second;
		ConfirmSFX = GameInstance->AudioManager->Play({ "Global/UI/Confirm" }, false);

		InitEvents(Pair.first);
	}
	else
	{
		ContinueSFX = GameInstance->AudioManager->Play({ "Global/UI/Dialogue/Continue" }, false);
	}

	StoryComponent->Advance();

	// Call all events
	for (int j = 0; j < Events.Num(); j++)
	{
		UDialogueOptionEventComponent::OnDialogueOptionEventDelegate.Broadcast(Events[j]);
	}

	return true;
}

void UDialogueWidget::Skip()
{
	SkipText = true;

	if (!OptionShown && StoryComponent)
	{
		if (StoryComponent->CheckNext())
		{
			OptionShown = true;
		}
	}

	if (Options.Num() > 0)
	{
		// std::pair<uint32 ID, uint32 NextID>
		NextDialogID = Options[CurrentMenuOption].second.second;
	}
}

void UDialogueWidget::SetStoryComponent(UStoryComponent* Component)
{
	StoryComponent = Component;
}

void UDialogueWidget::InitEvents(uint32 DialogueOptionID)
{
	// Search dialogue events for dialogue id
	auto Links = TableDatabase::Get().FindLinks("dialogueOptionEvents", "dialogueOptionId", DialogueOptionID);
	Events.Init(FDialogueOptionEvent(), Links.Num());

	// Create all events
	uint32 i = 0;
	for (auto& Link : Links)
	{
		bool NameFound = false;
		bool ValueFound = false;

		std::string s = "";
		double d = 0;

		for (auto& Field : Link.Value->Fields)
		{
			if (Field.Key.ColumnName == "name")
			{
				NameFound = true;

				s.resize(Field.Value.Size);
				memcpy(const_cast<char*>(s.c_str()), Field.Value.Data.Get(), Field.Value.Size);
			}
			else if (Field.Key.ColumnName == "value")
			{
				ValueFound = true;
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);
			}

			if (NameFound && ValueFound)
			{
				break;
			}
		}

		ensure(NameFound && ValueFound);

		Events[i].DialogueOptionID = DialogueOptionID;
		Events[i].CharacterID = StoryComponent->GetLinkedID();
		Events[i].EventName = s.c_str();
		Events[i].Value = d;
		i++;
	}
}

void UDialogueWidget::OnAnimationFinish_Implementation()
{
	AnimateText = false;

	if (ScrollSFX)
		ScrollSFX->Stop();
}
