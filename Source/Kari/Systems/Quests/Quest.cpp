#include "Quest.h"

#include <string>
#include "TableRow.h"
#include "TableObject.h"

FQuest FQuest::ConvertRow(const FTableRow& Row, int32 ID)
{
	FQuest Quest;
	Quest.ID = ID;

	if (Row.Fields.Num() == 0)
	{
		return Quest;
	}

	int32 childId = -INT_MAX;

	for (auto& field : Row.Fields)
	{
		if (field.Key == "title")
		{
			std::string title;
			title.resize(field.Value.Size);

			memcpy((void*)title.c_str(), field.Value.Data.Get(), field.Value.Size);
			Quest.Title = UTF8_TO_TCHAR(title.c_str());
		}

		else if (field.Key == "description")
		{
			std::string desc;
			desc.resize(field.Value.Size);

			memcpy((void*)desc.c_str(), field.Value.Data.Get(), field.Value.Size);
			Quest.Description = FText::FromString(UTF8_TO_TCHAR(desc.c_str()));
		}

		else if (field.Key == "moneyReward")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Quest.MoneyReward = d;
		}

		else if (field.Key == "type")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Quest.Type = static_cast<EQuestType>((int32)d);
		}

		else if (field.Key == "childId")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			childId = (int32)d;
		}
	}

	if (childId < 0)
		return Quest;

	FTask Task;
	Task.NextID = childId;

	UTableObject* TableObject = NewObject<UTableObject>();
	
	while (Task.NextID >= 0 && Task.ID != Task.NextID)
	{
		const FTableRow& NextTask = TableObject->GetRow("tasks", Task.NextID);
		
		if (NextTask.Fields.Num() == 0)
			break;

		Task = FTask::ConvertRow(NextTask, Task.NextID);
		Quest.Tasks.Add(Task);
	}

	return Quest;
}

void FQuest::Save(FArchive &Ar)
{
	Ar << ID; //int32
	Ar << Title; //string
	Ar << Description; //text
	Ar << MoneyReward; //int32
	Ar << Type; //int8
	Ar << Completed; //bool

	// Save tasks
	int32 Count = Tasks.Num();
	Ar << Count;
	for (auto& Task : Tasks)
	{
		Task.SaveLoad(Ar);
	}
}

void FQuest::Load(FArchive &Ar)
{
	Ar << ID; //int32
	Ar << Title; //string
	Ar << Description; //text
	Ar << MoneyReward; //int32
	Ar << Type; //int8
	Ar << Completed; //bool

	// Load tasks
	int32 Count;
	Ar << Count;
	for (int32 i=0; i<Count; i++)
	{
		FTask Task;
		Task.SaveLoad(Ar);
		Tasks.Add(Task);
	}
}