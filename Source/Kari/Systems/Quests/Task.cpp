#include "Task.h"

#include "TableRow.h"
#include <string>

FTask FTask::ConvertRow(const FTableRow& Row, int32 ID)
{
	FTask Task;
	Task.ID = ID;

	if (Row.Fields.Num() == 0)
	{
		return Task;
	}

	for (auto& field : Row.Fields)
	{
		if (field.Key == "description")
		{
			std::string desc;
			desc.resize(field.Value.Size);

			memcpy((void*)desc.c_str(), field.Value.Data.Get(), field.Value.Size);
			Task.Description = UTF8_TO_TCHAR(desc.c_str());
		}

		else if (field.Key == "requiredCount")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Task.RequiredCount = d;
		}

		else if (field.Key == "typeId")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Task.Type = static_cast<ETaskType>((int32)d);
		}

		else if (field.Key == "nextId")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Task.NextID = d;
		}

		if (field.Key == "npc")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Task.NPC = d;
		}

		else if (field.Key == "parentId")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Task.ParentID = d;
		}
		
		else if (field.Key == "childId")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Task.ChildID = d;
		}

		else if (field.Key == "hiddenAtFirst")
		{
			bool b;
			memcpy((void*)&b, field.Value.Data.Get(), field.Value.Size);

			Task.HiddenAtFirst = b;
		}
	}

	return Task;
}

void FTask::SaveLoad(FArchive &Ar)
{
	Ar << ID; //int32
	Ar << Description; //string
	Ar << CurrentCount; //int32
	Ar << RequiredCount; //int32
	Ar << Type; //int8
	Ar << NextID; //int32
	Ar << NPC; // uint32
	Ar << ParentID; // int32
	Ar << ChildID; //int32
	Ar << HiddenAtFirst; // bool
	Ar << Active; // bool
	Ar << Completed; // bool
	Ar << Frozen; // bool
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Description);
}