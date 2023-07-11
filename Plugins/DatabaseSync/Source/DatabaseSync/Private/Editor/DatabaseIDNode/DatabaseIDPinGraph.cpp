#if WITH_EDITOR
#include "DatabaseIDPinGraph.h"
#include "STextComboBox.h"
#include "TableDatabase.h"

void SDatabaseIDPinGraph::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	EntryNames.Empty();
	TableNames.Empty();
	ColumnNames.Empty();
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget>	SDatabaseIDPinGraph::GetDefaultValueWidget()
{
	TableID = 0;
	ColumnID = 0;
	EntryID = 0;
	FString CurrentDefault = GraphPinObj->GetDefaultAsString();

	CreateTableDropdownValues();
	if (CurrentDefault.Len() > 0 && CurrentDefault.Contains("TableName"))
	{
		int32 StartIndex;
		StartIndex = CurrentDefault.Find("TableName=\"") + 11;
		int32 EndIndex;
		EndIndex = CurrentDefault.Find("\"", ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex) - StartIndex;

		FString DefaultValueString = CurrentDefault.Mid(StartIndex, EndIndex);
		for (int i = 0; i < TableNames.Num(); i++)
		{
			if (DefaultValueString == *TableNames[i])
			{
				TableID = i;
				break;
			}
		}
	}

	CreateColumnDropdownValues();
	if (CurrentDefault.Len() > 0 && CurrentDefault.Contains("ColumnName"))
	{
		int32 StartIndex;
		StartIndex = CurrentDefault.Find("ColumnName=\"") + 12;
		int32 EndIndex;
		EndIndex = CurrentDefault.Find("\"", ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex) - StartIndex;

		FString DefaultValueString = CurrentDefault.Mid(StartIndex, EndIndex);
		for (int i = 0; i < ColumnNames.Num(); i++)
		{
			if (DefaultValueString == *ColumnNames[i])
			{
				ColumnID = i;
				break;
			}
		}
	}

	CreateEntryDropdownValues();
	if (CurrentDefault.Len() > 0 && CurrentDefault.Contains("ID"))
	{
		int32 StartIndex;
		StartIndex = CurrentDefault.Find("ID=") + 3;
		int32 EndIndex;
		EndIndex = CurrentDefault.Find(")", ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex) - StartIndex;

		FString DefaultValueString = CurrentDefault.Mid(StartIndex, EndIndex);
		int32 ID = FCString::Atoi(*DefaultValueString);

		const int32* EntryIDPtr = EntryIDMapping.FindKey(ID);
		if (EntryIDPtr != nullptr)
			EntryID = *EntryIDPtr;
		if (EntryID >= EntryNames.Num())
			EntryID = 0;
	}

	// Create widget
	return SNew(SHorizontalBox)	+
		SHorizontalBox::Slot().AutoWidth().MaxWidth(100.0f).HAlign(HAlign_Left) [
			SAssignNew(TableWidget, STextComboBox)
				.OptionsSource(&TableNames)
				.OnSelectionChanged(this, &SDatabaseIDPinGraph::OnTableSelected)
				.InitiallySelectedItem(TableNames[TableID])
		] +
		SHorizontalBox::Slot().AutoWidth().MaxWidth(100.0f).HAlign(HAlign_Left) [
			SAssignNew(ColumnWidget, STextComboBox)
				.OptionsSource(&ColumnNames)
				.OnSelectionChanged(this, &SDatabaseIDPinGraph::OnColumnSelected)
				.InitiallySelectedItem(ColumnNames[ColumnID])
		] +
		SHorizontalBox::Slot().AutoWidth().MaxWidth(100.0f).HAlign(HAlign_Left) [
			SAssignNew(EntryWidget, STextComboBox)
				.OptionsSource(&EntryNames)
				.OnSelectionChanged(this, &SDatabaseIDPinGraph::OnEntrySelected)
				.InitiallySelectedItem(EntryNames[EntryID])
		];
}

void SDatabaseIDPinGraph::OnTableSelected(TSharedPtr<FString> Selected, ESelectInfo::Type SelectInfo)
{
	
	if (Selected.IsValid())
	{
		for (int32 i = 0; i < TableNames.Num(); ++i)
		{
			if (TableNames[i] == Selected)
			{
				TableID = i;
			}
		}
	}

	ColumnID = 0;
	EntryID = 0;
	CreateColumnDropdownValues();
	CreateEntryDropdownValues();
	
	ColumnWidget->SetSelectedItem(ColumnNames[ColumnID]);
	EntryWidget->SetSelectedItem(EntryNames[EntryID]);
	SetValue();

	TableWidget->RefreshOptions();
	ColumnWidget->RefreshOptions();
	EntryWidget->RefreshOptions();
}

void SDatabaseIDPinGraph::OnColumnSelected(TSharedPtr<FString> Selected, ESelectInfo::Type SelectInfo)
{
	if (Selected.IsValid())
	{
		for (int32 i = 0; i < ColumnNames.Num(); ++i)
		{
			if (ColumnNames[i] == Selected)
			{
				ColumnID = i;
			}
		}
	}

	EntryID = 0;
	CreateEntryDropdownValues();

	ColumnWidget->SetSelectedItem(ColumnNames[ColumnID]);
	EntryWidget->SetSelectedItem(EntryNames[EntryID]);
	SetValue();

	TableWidget->RefreshOptions();
	ColumnWidget->RefreshOptions();
	EntryWidget->RefreshOptions();
}

void SDatabaseIDPinGraph::OnEntrySelected(TSharedPtr<FString> Selected, ESelectInfo::Type SelectInfo)
{
	if (Selected.IsValid())
	{
		for (int32 i = 0; i < EntryNames.Num(); ++i)
		{
			if (EntryNames[i] == Selected)
			{
				EntryID = i;
			}
		}
	}

	SetValue();

	TableWidget->RefreshOptions();
	ColumnWidget->RefreshOptions();
	EntryWidget->RefreshOptions();
}

void SDatabaseIDPinGraph::SetValue()
{
	FString StrID;
	StrID.AppendInt(EntryIDMapping.FindRef(EntryID));

	FString SetString = TEXT("(");
	SetString += TEXT("TableName=\"");
	SetString += *TableNames[TableID];
	SetString += TEXT("\",");

	SetString += TEXT("ColumnName=\"");
	SetString += *ColumnNames[ColumnID];
	SetString += TEXT("\",");

	SetString += TEXT("ID=");
	SetString += StrID;
	SetString += TEXT(")");

	GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, SetString);
}

void SDatabaseIDPinGraph::CreateTableDropdownValues()
{
	TableNames.Empty();

	// Create dropdown entries for table selection
	const TMap<FString, TableBinary>& Tables = TableDatabase::Get().GetBinaries();
	for (auto& TableBinary : Tables)
	{
		TableNames.Add(MakeShareable<FString>(new FString(TableBinary.Key)));
	}
}

void SDatabaseIDPinGraph::CreateColumnDropdownValues()
{
	ColumnNames.Empty();

	// Create dropdown entries for column selection
	const FTableRow Row = TableDatabase::Get().GetRow(*TableNames[TableID], 0);
	for (auto& Field : Row.Fields)
	{
		// Filter out internal colums
		if (Field.Key.ColumnName == "deleted" || 
			Field.Key.ColumnName == "tstamp")
			continue;

		ColumnNames.Add(MakeShareable<FString>(new FString(Field.Key.ColumnName)));
	}
}

void SDatabaseIDPinGraph::CreateEntryDropdownValues() 
{
	EntryNames.Empty();

	// Try etermin entry type
	int EntryType = 0; // 0:string, 1:double, 2:bool
	const FTableField* TypeField = TableDatabase::Get().GetField(*TableNames[TableID], *ColumnNames[ColumnID], 0);
	if (TypeField->Size == 1)
	{
		// A field with size == 1 and value == 0|1 must be a boolean
		if (*TypeField->Data == 0 || *TypeField->Data == 1)
			EntryType = 2;

	} else if (TypeField->Size == 8)
	{
		// If each char is a printable character we interpret this data as string.
		// Warning: this method is not full proof and will falsly determin certain combinations as string, but currently there is not better distinction.
		bool IsString = true;
		for (int i = 0; i < 8; i++)
		{
			if (!isprint(TypeField->Data.Get()[i]))
				IsString = false;
		}

		if (!IsString)
			EntryType = 1;
	}

	// Create dropdown entries from database
	int rowCount = TableDatabase::Get().GetRows(*TableNames[TableID]).Num();
	for (int j = 0; j < rowCount; j++)
	{
		const FTableField* DeletedField = TableDatabase::Get().GetField(*TableNames[TableID], "deleted", j);
		if (DeletedField == nullptr)
			continue;
		ensure(DeletedField);

		bool Deleted = false;
		memcpy((void*)&Deleted, DeletedField->Data.Get(), DeletedField->Size);

		const FTableField* Field = TableDatabase::Get().GetField(*TableNames[TableID], *ColumnNames[ColumnID], j);

		// There is a valid scenario where Field == nullptr, we wnat to skip this scenario.
		if (Deleted || Field == nullptr) 
			continue;

		if (EntryType == 0) // string
		{
			std::string s;
			s.resize(Field->Size);
			memcpy((void*)s.c_str(), Field->Data.Get(), Field->Size);

			EntryNames.Add(MakeShareable<FString>(new FString(s.c_str())));
		} else if (EntryType == 1) // double
		{
			double d;
			memcpy((void*)&d, Field->Data.Get(), Field->Size);

			EntryNames.Add(MakeShareable<FString>(new FString(FString::SanitizeFloat(d, 0))));
		} else if (EntryType == 2) // bool
		{
			bool b;
			memcpy((void*)&b, Field->Data.Get(), Field->Size);

			if (b)
				EntryNames.Add(MakeShareable<FString>(new FString("true")));
			else
				EntryNames.Add(MakeShareable<FString>(new FString("false")));
		}

		EntryIDMapping.Add(EntryNames.Num() - 1, j);
	}
}
#endif