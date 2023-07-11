#include "StoryComponent.h"
#include "UserWidget.h"

#include "Kismet/GameplayStatics.h"

#include "RichTextBlock.h"
#include "UI/Dialogues/DialogueWidget.h"

#include "TableDatabase.h"
#include "TableBinary.h"
#include "Systems/Components/DialogueEventComponent.h"
#include "Systems/Revisioning/Revisioning.h"
#include "Characters/NPCs/DefaultNPC.h"

#include "General/DefaultGameInstance.h"
#include "Characters/Playables/DefaultPlayer.h"
#include "Systems/Quests/QuestLog.h"

UStoryComponent::UStoryComponent() : UTableComponent("stories", "title", "parentId")
{
	DropdownDisabled = true;
}

void UStoryComponent::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UDefaultGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	AudioManager = GameInstance->AudioManager;

	if (MusicFaderActive)
	{
		if (!MusicFader)
			MusicFader = NewObject<UBGMFader>(this);

		if (MusicFader)
			MusicFader->Init(
				BGMPath,
				FadeTime,
				RevertOnOverlapEnd,
				ResetOnExit,
				Loop,
				Shuffle,
				Volume,
				LoopStart,
				LoopEnd,
				PitchMin,
				PitchMax,
				Delay
			);
	}
}

void UStoryComponent::SetPendingStoryID(int32 StoryID)
{
	PendingStoryID = StoryID;
}

void UStoryComponent::AddPendingQuestID(int32 QuestID)
{
	PendingQuestIDS.AddUnique(QuestID);	
}

bool UStoryComponent::ShowDialogue(int32 dialogueId)
{
	if (Shown || ID < 0)
		return false;

	FStringClassReference WidgetReference(TEXT("/Game/UI/Dialogues/WBP_DialogueWidget.WBP_DialogueWidget_C"));
	UClass* WidgetClass = WidgetReference.TryLoadClass<UDialogueWidget>();

	if (WidgetClass)
	{
		UInteractableComponent* Interactable = Cast<UInteractableComponent>(GetOwner()->GetComponentByClass(UInteractableComponent::StaticClass()));
		ensure(Interactable);

		// Only look for GodWidgets when have widgets at all.
		DialogueWidget = CreateWidget<UDialogueWidget>(
			UGameplayStatics::GetPlayerController(GetWorld(), 0),
			*CustomWidgetOverride ? (*CustomWidgetOverride) : WidgetClass
		);				

		if (DialogueWidget)
		{
			DialogueWidget->SetStoryComponent(this);
			SetStory(ID, dialogueId == -1 ? -INT_MAX : dialogueId );

			DialogueWidget->AddToViewport();

			if (ShouldCameraZoom)
				Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->GetCameraSystem()->Zoom();

			// Play accompanied BGM
			if (MusicFaderActive && MusicFader)
				MusicFader->Apply();

			Shown = true;

			ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

			Player->SetInteractionCharacterID(CharacterComponent->GetTableID());

			return true;
		}
	}

	return false;
}

void UStoryComponent::HideDialogue()
{
	Shown = false;

	// Resets chetaracter ID
	ADefaultPlayer* Player = Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	Player->SetInteractionCharacterID(-INT_MAX);


	if (ShouldCameraZoom)
	{
		Cast<ADefaultPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->GetCameraSystem()->Unzoom();
	}
		
	// Revert BGM
	if (MusicFaderActive && MusicFader)
		MusicFader->Revert();

	for (int i = 0; i < Events.Num(); i++)
	{
		UDialogueEventComponent::OnDialogueEventDelegate.Broadcast(Events[i], false);
	}
	Events.Empty();

	UInteractableComponent* Interactable = Cast<UInteractableComponent>(GetOwner()->GetComponentByClass(UInteractableComponent::StaticClass()));
	ensure(Interactable);
	Interactable->OnInteractEnd.Broadcast();

	if (PendingStoryID != -INT_MAX)
	{
		SetStory(PendingStoryID);
		PendingStoryID = -INT_MAX;
	}

	for(const auto& QuestID: PendingQuestIDS)
	{
		GameInstance->QuestLog->AddQuest(QuestID, CharacterComponent ? CharacterComponent->GetTableID() : -1);
	}
	PendingQuestIDS.Empty();

	if (!DialogueWidget)
		return;

	DialogueWidget->StopSFX();
	DialogueWidget->RemoveFromParent();
	DialogueWidget = nullptr;
}

void UStoryComponent::Advance()
{
	if (!DialogueWidget)
		return;

	const int32 NextDialogID = DialogueWidget->GetNextDialogID();

	if (NextDialogID < 0)
	{
		HideDialogue();
	}
	else
	{
		SetDialogue(NextDialogID);
	}
}

bool UStoryComponent::CheckNext()
{
	if (NextID >= 0)
		return false;

	return CheckDialogueOptions();
}

void UStoryComponent::SetStory(int32 storyId, int32 dialogueId)
{
	const FTableField* story = GetField("stories", "childId", storyId);

	if (!story || story->Size == 0)
	{
		DialogueWidget->SetDialogue(
			FText::FromString(""),
			"",
			storyId,
			-INT_MAX
		);

		NextID = -INT_MAX;

		return;
	}

	// If dialogue is not set override it with the story childId
	double DialogueId = dialogueId;
	if(dialogueId < 0)
		memcpy(&DialogueId, story->Data.Get(), story->Size);

	SetDialogue(DialogueId);
}

void UStoryComponent::SetDialogue(int32 dialogueId)
{
	for (int i = 0; i < Events.Num(); i++)
	{
		UDialogueEventComponent::OnDialogueEventDelegate.Broadcast(Events[i], false);
	}
	Events.Empty();

	// Search dialogue events for dialogue id
	auto Links = TableDatabase::Get().FindLinks("dialogueEvents", "dialogueId", dialogueId);
	Events.Init(FDialogueEvent(), Links.Num());

	// Create all events
	uint32 i = 0;
	for (auto& Link : Links)
	{
		bool NameFound = false;
		bool ValueFound = false;
		bool SValueFound = false;

		std::string s = "";
		double d = 0;
		double d2 = 0;

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
			else if (Field.Key.ColumnName == "secondaryValue")
			{
				SValueFound = true;
				memcpy(&d2, Field.Value.Data.Get(), Field.Value.Size);
			}

			if (NameFound && ValueFound && SValueFound)
			{
				break;
			}
		}

		ensure(NameFound && ValueFound && SValueFound);

		const FString EventName = s.c_str(); 
		
		Events[i].DialogueID = dialogueId;
		Events[i].CharacterID = CharacterComponent->GetTableID();
		Events[i].EventName = EventName;
		Events[i].Value = d;
		Events[i].SecondaryValue = d2;

		i++;
	}

	// Call all events --> changed it to j so that it doesn't hide previous declaration.
	for (int j = 0; j < Events.Num(); j++)
	{
		UDialogueEventComponent::OnDialogueEventDelegate.Broadcast(Events[j], true);
	}

	// Set new text
	const FTableField* text = GetField("dialogues", "text", dialogueId);
	ensure(text);

	if (!text)
	{
		HideDialogue();
		return;
	}

	const FTableField* charID = GetField("dialogues", "characterId", dialogueId);
	ensure(charID);

	if (!charID)
	{
		HideDialogue();
		return;
	}

	const FTableField* parentId = GetField("dialogues", "parentId", dialogueId);
	ensure(parentId);

	double parent = GetTableID();

	if (!parentId)
	{
		HideDialogue();
		return;
	}

	if (parentId->Size != 0)
		memcpy(&parent, parentId->Data.Get(), parentId->Size);

	double d = -INT_MAX;
	std::string n = "";

	if (charID->Size != 0)
	{
		memcpy(&d, charID->Data.Get(), charID->Size);

		const FTableField* name = GetField("characters", "name", d);
		ensure(name);

		if (name)
		{
			n.resize(name->Size);

			if (name && name->Size != 0)
				memcpy(const_cast<char*>(n.c_str()), name->Data.Get(), name->Size);
		}
	}

	const FTableField* nextId = GetField("dialogues", "nextId", dialogueId);
	
	std::string s;
	s.resize(text->Size);

	d = -INT_MAX;

	if (text->Size != 0)
		memcpy(const_cast<char*>(s.c_str()), text->Data.Get(), text->Size);

	if (nextId && nextId->Size != 0)
		memcpy(&d, nextId->Data.Get(), nextId->Size);

	CurrentID = dialogueId;
	NextID = d;

	DialogueWidget->SetDialogue(
		FText::FromString(
			UTF8_TO_TCHAR(s.c_str())
		),
		UTF8_TO_TCHAR(n.c_str()),
		dialogueId,
		d
	);

	// Change to story id of dialogue
	if (parent != ID && CharacterComponent)
	{
		FCharacterRevision Revision;
		Revision.StoryID = parent;

		GameInstance->Revisioning->SetCharacterRevision(CharacterComponent->GetTableID(), Revision);
	}
}

bool UStoryComponent::CheckDialogueOptions()
{
	TArray<TPair<uint32, const FTableRow*>> dialogueOptions = FindLinks("dialogueOptions", "parentId", CurrentID);

	if (dialogueOptions.Num() == 0)
		return false;

	for (auto& dialogueOption : dialogueOptions)
	{
		std::string s;
		double id = -INT_MAX;

		uint32 i = 0;

		for (auto& field : dialogueOption.Value->Fields)
		{
			bool isText = field.Key == "text";
			bool isNext = field.Key == "childId";

			if (!isText && !isNext)
				continue;

			if (field.Value.Size == 0)
				continue;

			if (isText)
			{
				s.resize(field.Value.Size);
				memcpy(const_cast<char*>(s.c_str()), field.Value.Data.Get(), field.Value.Size);
				i++;
			}
			else
			{
				memcpy(&id, field.Value.Data.Get(), FMath::Min((size_t)sizeof(double), (size_t)field.Value.Size));
				i++;
			}

			if (i == 2)
				break;
		}

		DialogueWidget->AddOption(s, { dialogueOption.Key, id });
	}

	DialogueWidget->OnOptionsAddedComplete();

	return true;
}
