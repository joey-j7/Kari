#include "QuestLog.h"

#include <Engine.h>

#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/FileHelper.h"

#include "General/DefaultGameInstance.h"
#include "Systems/Components/TaskEventComponent.h"
#include "Systems/Components/InteractableComponent.h"
#include "Systems/Revisioning/Revisioning.h"
#include "Systems/Inventory/Inventory.h"
#include "UI/HUD/Log/HUDLogWidget.h"
#include "Quest.h"
#include "Events/DefaultEvent.h"
#include "TableDatabase.h"

UQuestLog::UQuestLog() : UTableComponent("quests", "title")
{

}
TArray<FQuest> UQuestLog::GetQuests() const
{
	TArray<FQuest> quests;

	for (auto& Elem : Quests)
	{
		quests.Add(Elem.Value);
	}
	
	return quests;
}

void UQuestLog::AddQuest(int32 QuestID, int32 CharacterID)
{
	FQuest* Q = Quests.Find(QuestID);

	if (Q)
	{
		ActiveQuests.AddUnique(QuestID);
		ActiveQuests.Sort();

		// This is mainly used for events, in order to set the name of the quest giver in the UI.
		if(CharacterID != -1)
			ActiveCharacterIds.FindOrAdd(QuestID, CharacterID);
		
		return;
	}

	const FTableRow& Row = GetRow("quests", QuestID);

	if (Row.Fields.Num() <= 0)
	{
	#if WITH_EDITOR
		UE_LOG(LogTemp, Fatal, TEXT("Quest does not exist, is corrupted or empty!"));
	#endif

		return;
	}

	const FQuest Quest = FQuest::ConvertRow(Row, QuestID);

	bool SetActive = true;

	ActiveQuests.AddUnique(QuestID);
	ActiveQuests.Sort();
	
	Quests.Add(QuestID, Quest);

	// This is mainly used for events, in order to set the name of the quest giver in the UI.
	if(CharacterID >= 0)
		ActiveCharacterIds.FindOrAdd(QuestID, CharacterID);

	bool Completed = true;

	// Update inventory count, etc.
	for (auto& T : Quests[QuestID].Tasks)
	{
		FTask& Task = const_cast<FTask&>(T);

		// Update active state
		if (SetActive && !Task.Active && !Task.Completed)
		{
			Task.Active = true;
			OnTaskEnter(Task);
		}

		// Update completion state
		UpdateTask(Task);

		if (!Task.Completed)
		{
			Completed = false;
		}

		// Don't activate tasks from hereon
		if (!Task.Completed || Task.HiddenAtFirst)
		{
			SetActive = false;
		}
	}

	if (Quest.ID >= 0 && Completed)
	{
		CompleteQuest(Quest.ID);
		return;
	}

	if (Instance)
	{
		// Notify the user that we have a active task.
		Instance->GetHUDLog()->AddMessage(Quest.ID, "New quest added", EMessageType::E_QUEST);;
	}
}

void UQuestLog::CompleteQuest(int32 QuestID)
{
	if (ActiveQuests.Find(QuestID) == INDEX_NONE)
		return;

	const FTableRow& Row = GetRow("quests", QuestID);

	if (Row.Fields.Num() <= 0)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Fatal, TEXT("Quest does not exist, is corrupted or empty!"));
#endif

		return;
	}

	auto Links = FindLinks("tasks", "parentId", QuestID);

	if (Links.Num() == 0)
	{
		FQuest* Quest = Quests.Find(QuestID);

		if (!Quest)
			return;

		Quest->Completed = true;
		OnQuestCompleted(*Quest);
	}
	
	for (auto& Link : Links)
	{
		CompleteTask(Link.Key);
	}
}

void UQuestLog::MarkQuestAsActive(int32 QuestID)
{
	int32 i = ActiveQuests.Find(QuestID);

	if (i == INDEX_NONE)
		return;

	ActiveQuests.RemoveAt(i);
	
	AddQuest(QuestID);
}

void UQuestLog::ResetQuest(int32 QuestID)
{
	const FQuest* Find = FindQuest(QuestID);

	if (!Find)
		return;

	Quests.Remove(QuestID);
	ActiveQuests.Remove(QuestID);

	const FTableRow& Row = GetRow("quests", QuestID);

	if (Row.Fields.Num() <= 0)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Fatal, TEXT("Quest does not exist, is corrupted or empty!"));
#endif

		return;
	}

	const FQuest Quest = FQuest::ConvertRow(Row, QuestID);

	if (Instance)
	{
		Instance->GetHUDLog()->AddMessage(Quest.ID, "New quest added", EMessageType::E_QUEST);
	}
}

void UQuestLog::CompleteTask(int32 TaskID)
{
	bool IsCompleted = false;
	std::pair<int32, bool> TaskCompleted = { -1, false };
	FQuest* Q = nullptr;

	for (auto& Quest : Quests)
	{
		IsCompleted = true;

		for (auto& Task : Quest.Value.Tasks)
		{
			if (Task.ID == TaskID && !Task.Completed)
			{
				Q = &Quest.Value;

				Task.Active = false;
				Task.Completed = true;
				TaskCompleted.second = true;
				
				OnTaskCompleted(Task);

				// Mark next task as active
				UpdateActiveTasks();
			}

			if (!Task.Completed)
			{
				IsCompleted = false;
			}
		}

		if (Q && Q->ID >= 0)
			break;
	}

	if (Q && Q->ID >= 0 && IsCompleted)
	{
		Q->Completed = true;
		OnQuestCompleted(*Q);
	}
}

void UQuestLog::AddRevisionIDChangeToTask(int32 TaskID, int32 CharacterID, int32 StoryID)
{
	for (auto& Q : Quests)
	{
		FQuest* Find = &Q.Value;

		// Reverse lookup to find task to freeze
		for (int32 j = Find->Tasks.Num() - 1; j >= 0; --j)
		{
			FTask& Task = Find->Tasks[j];

			if (Task.ID == TaskID)
			{
				Task.RevertIDs.Add(CharacterID, StoryID);
				break;
			}
		}
	}
}

void UQuestLog::FreezeTask(int32 TaskID)
{
	for (auto& Q : Quests)
	{
		FQuest* Find = &Q.Value;

		int32 TaskIndex = -1;
		bool IsEverythingBeforeCompleted = true;

		// Reverse lookup to find task to freeze
		for (int32 j = Find->Tasks.Num() - 1; j >= 0; --j)
		{
			FTask& Task = Find->Tasks[j];

			if (Task.ID != TaskID)
			{
				if (TaskIndex >= 0)
				{
					if (!Task.Completed)
					{
						IsEverythingBeforeCompleted = false;
						break;
					}
				}

				continue;
			}

			// Already frozen?
			if (Task.Frozen)
				break;

			TaskIndex = j;
		}

		// Only if tasks before to be frozen task are completed
		if (TaskIndex >= 0 && IsEverythingBeforeCompleted)
		{
			for (int32 j = 0; j < Find->Tasks.Num(); ++j)
			{
				FTask& Task = Find->Tasks[j];

				if (Task.ID == TaskID)
				{
					Task.Frozen = true;
					break;
				}

				Task.Frozen = true;
			}
		}
	}
}

void UQuestLog::UpdateActiveTasks()
{
	if (!Instance)
		return;

	for (int i = 0; i < ActiveQuests.Num(); ++i)
	{
		const auto QuestID = ActiveQuests[i];
		FQuest* Find = Quests.Find(QuestID);

		if (!Find)
			continue;

		bool SetActive = true;

		for (auto& Task : Find->Tasks)
		{
			// Update active state
			if (SetActive && !Task.Active && !Task.Completed)
			{
				Task.Active = true;
				OnTaskEnter(Task);
			}

			if (!SetActive && Task.Active)
			{
				Task.Active = false;
				OnTaskExit(Task);
				Task.RevertIDs.Empty();
			}

			// Update completion state
			UpdateTask(Task);

			// Don't activate tasks from hereon
			if (!Task.Completed)
			{
				if (SetActive)
				{
					Task.Active = true;
				}

				SetActive = false;
			}
		}
	}
}

void UQuestLog::OnInteractWithTask(UInteractableComponent* Interactable)
{
	for (int32 i : ActiveQuests)
	{
		const FQuest* Q = FindQuest(i);

		if (!Q)
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid quest spotted in active quest array!"))
			continue;
		}

		for (auto& T : Q->Tasks)
		{
			FTask& Task = const_cast<FTask&>(T);
			switch (Task.Type)
			{
			case E_INTERACT:
				// TODO maybe add current amount if you have to talk with multiple NPCs

				// If we have the same ID
				if (Task.NPC == Interactable->GetTableID() /* && Task.CurrentCount == Task.RequiredCount */)
				{
					// Complete the task
					if (!Task.Completed && !Interactable->HasInteracted)
					{
						CompleteTask(Task.ID);
						// Now we already interacted with the NPC
						Interactable->HasInteracted = true;
					}
				}
				else if (!Task.Frozen && Task.Completed)
				{
					Task.Completed = false;
				}

				break;
			default:
				break;
			}
		}
	}
}

void UQuestLog::OnTaskEnter_Implementation(FTask Task)
{
	if (!Task.Active)
		return;

	TArray<TPair<uint32, const FTableRow*>> Links = FindLinks("taskEvents", "taskId", Task.ID);
	for (auto& Link : Links)
	{
        std::string s;
		double d = DBL_MAX;
		double c = DBL_MAX;

		for (auto& Field : Link.Value->Fields)
		{
			// Get event name
			if (Field.Key.ColumnName == "name")
			{
				s.resize(Field.Value.Size);
                memcpy((void*)s.c_str(), Field.Value.Data.Get(), Field.Value.Size);
			}

			// Get event value
			if (Field.Key.ColumnName == "value")
			{
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);
			}

			// Get event value
			if (Field.Key.ColumnName == "characterId")
			{
				memcpy(&c, Field.Value.Data.Get(), Field.Value.Size);
			}
		}

		FTaskEvent CurrentTaskEvent;

		CurrentTaskEvent.TaskID = Task.ID;
		CurrentTaskEvent.CharacterID = c;
		CurrentTaskEvent.EventName = FString(s.c_str());
		CurrentTaskEvent.Value = d;
		
		UTaskEventComponent::OnTaskEventDelegate.Broadcast(CurrentTaskEvent, true);
	}

	// Notify the user that we have a active task.
	Instance->GetHUDLog()->AddMessage(Task.ID, Task.Description, EMessageType::E_TASK);
}

void UQuestLog::OnTaskExit_Implementation(FTask Task)
{
	if (Task.Active)
		return;

	for (auto& TaskID : Task.RevertIDs)
	{
		const int32 CharacterID = TaskID.Key;
		const int32 StoryID = TaskID.Value;

		FCharacterRevision Revision;
		Revision.StoryID = StoryID;

		Instance->Revisioning->SetCharacterRevision(CharacterID, Revision);
	}
}

bool UQuestLog::SaveData()
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WriteData(ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\QuestSavedData.sav"));

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}

void UQuestLog::WriteData(FArchive &Ar)
{
	// Save the active quests
	int32 Count = ActiveQuests.Num();
	Ar << Count;
	for (auto& Quest : ActiveQuests)
	{
		Ar << Quest;
	}

	// Save the completed quests
	Count = CompletedQuests.Num();
	Ar << Count;
	for (auto& Quest : CompletedQuests)
	{
		Ar << Quest;
	}

	// Save the quests
	Count = Quests.Num();
	Ar << Count;
	for (auto& Quest : Quests)
	{
		Quest.Value.Save(Ar);
	}

	// Save the active character ids
	// Save the quests
	Count = ActiveCharacterIds.Num();
	Ar << Count;
	for (auto& ActiveID : ActiveCharacterIds)
	{
		Ar << ActiveID.Key; // int32
		Ar << ActiveID.Value; // int32
	}

	// Save money
	if (Instance)
	{
		int32 Money = Instance->GetMoney();
		Ar << Money;
	}
}

bool UQuestLog::LoadData()
{
	// Remove previous data
	Quests.Empty();
	ActiveQuests.Empty();
	ActiveCharacterIds.Empty();
	CompletedQuests.Empty();

	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\QuestSavedData.sav"))) return false;

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadData(FromBinary);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void UQuestLog::ReadData(FArchive &Ar)
{
	// Load the active quests
	int32 Count;
	Ar << Count;
	for (int32 i = 0; i < Count; i++)
	{
		int32 Quest;
		Ar << Quest;
		ActiveQuests.Add(Quest);
		ActiveQuests.Sort();
	}

	// Load the completed quests
	Ar << Count;
	for (int32 i = 0; i < Count; i++)
	{
		int32 Quest;
		Ar << Quest;
		CompletedQuests.Add(Quest);
	}

	// Load the quests
	Ar << Count;
	for (int32 i = 0; i < Count; i++)
	{
		FQuest Quest;
		Quest.Load(Ar);
		Quests.Add(Quest.ID,Quest);
	}

	// load the active character ids
	Ar << Count;
	for (int32 i = 0; i < Count; i++)
	{
		int32 KeyID;
		int32 ActiveID;
		Ar << KeyID;
		Ar <<  ActiveID;
		ActiveCharacterIds.Add(KeyID, ActiveID);
	}
}

const FQuest* UQuestLog::FindQuest(uint32 QuestID) const
{
	return Quests.Find(QuestID);
}

const int32* UQuestLog::FindCharacterID(uint32 CharacterID) const
{
	return ActiveCharacterIds.Find(CharacterID);
}

void UQuestLog::UpdateTask(FTask& Task)
{
	switch (Task.Type)
	{
	case E_COLLECT:
		Task.CurrentCount = FMath::Min(Instance->Inventory->CountItem(Task.ChildID), Task.RequiredCount);

		if (Task.CurrentCount == Task.RequiredCount)
		{
			if (!Task.Completed)
			{
				CompleteTask(Task.ID);
			}
		}
		else if (!Task.Frozen && Task.Completed)
		{
			Task.Completed = false;
		}

		break;
	default:
		break;
	}
}

void UQuestLog::OnTaskCompleted_Implementation(FTask Task)
{
	if (Task.Active)
		return;

	TArray<TPair<uint32, const FTableRow*>> Links = FindLinks("taskEvents", "taskId", Task.ID);
	for (auto& Link : Links)
	{
		char* s = nullptr;
		double d = DBL_MAX;
		double c = DBL_MAX;

		for (auto& Field : Link.Value->Fields)
		{
			// Get event name
			if (Field.Key.ColumnName == "name")
			{
				s = new char[Field.Value.Size];
                memcpy(s, Field.Value.Data.Get(), Field.Value.Size);
			}

			// Get event value
			if (Field.Key.ColumnName == "value")
			{
				memcpy(&d, Field.Value.Data.Get(), Field.Value.Size);
			}

			// Get event value
			if (Field.Key.ColumnName == "characterId")
			{
				memcpy(&c, Field.Value.Data.Get(), Field.Value.Size);
			}
		}

		FTaskEvent CurrentTaskEvent;

		CurrentTaskEvent.TaskID = Task.ID;
		CurrentTaskEvent.CharacterID = c;
		CurrentTaskEvent.EventName = FString(s);
		CurrentTaskEvent.Value = d;

		delete[] s; s = nullptr;
		
		UTaskEventComponent::OnTaskEventDelegate.Broadcast(CurrentTaskEvent, false);
	}
}

void UQuestLog::OnQuestCompleted_Implementation(FQuest Quest)
{
	if (Quest.ID < 0)
		return;

	ActiveQuests.Remove(Quest.ID);
	CompletedQuests.AddUnique(Quest.ID);

	if (Instance)
	{
		Instance->GetHUDLog()->AddMessage(Quest.ID, Quest.Title, EMessageType::E_QUEST);
		Instance->AddMoney(Quest.MoneyReward);
	}
}
