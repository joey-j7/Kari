#include "TaskEventComponentDetails.h"
#if WITH_EDITOR
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "SBoxPanel.h"
#include "STextComboBox.h"
#include "DetailWidgetRow.h"
#include "TableDatabase.h"
#include "Systems/Components/TaskEventComponent.h"

void FTaskEventComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const TArray<TWeakObjectPtr<UObject>>& SelectedObjects = DetailBuilder.GetDetailsView()->GetSelectedObjects();

	// Find our component
	for (int32 ObjectIndex = 0; !TaskEventComponent.IsValid() && ObjectIndex < SelectedObjects.Num(); ObjectIndex++)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			TaskEventComponent = Cast<UTaskEventComponent>(CurrentObject.Get());
		}
	}

	// Create dropdown entries from database
	EventNames.Empty();
	auto Rows = TableDatabase::Get().GetRows("taskEvents");
	for (auto& Row : Rows)
	{
		// Get data to check field validity.
		bool Deleted = true;
		FString Name;

		for (auto& Field : Row.Value.Fields)
		{
			if (Field.Key.ColumnName == "deleted")
			{
				memcpy((void*)&Deleted, Field.Value.Data.Get(), Field.Value.Size);
			} else if (Field.Key.ColumnName == "name")
			{
				std::string s;
				s.resize(Field.Value.Size);
				memcpy((void*)s.c_str(), Field.Value.Data.Get(), Field.Value.Size);
				Name = FString(s.c_str());
			}
		}

		if (!Deleted)
		{
			// Only add if the name is unique
			bool IsEqual = true;
			for (auto& EventName : EventNames)
			{
				if (EventName.Get()->Compare(Name) == 0)
				{
					IsEqual = false;
					break;
				}
			}

			if (IsEqual)
				EventNames.Add(MakeShareable<FString>(new FString(Name)));
		}
	}

	if (EventNames.Num() <= 0)
		return;

	// Find currently selected event name
	SelectedEventNameID = 0;
	if (TaskEventComponent->EventName.IsEmpty())
	{
		TaskEventComponent->EventName = *EventNames[0].Get();
	} else
	{
		for (int i = 0; i < EventNames.Num(); i++)
		{
			if (*EventNames[i].Get() == TaskEventComponent->EventName)
			{
				SelectedEventNameID = i;
				break;
			}
		}
	}

	// Create widget
	DetailBuilder.EditCategory("Task Event", FText::GetEmpty(), ECategoryPriority::Important)
		.AddCustomRow(FText::FromString("Task Event"), false).NameContent()
		[
			SNew(STextBlock)
				.Text(FText::FromString("Event Name"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MaxDesiredWidth(0.0f) // don't constrain the combo button width
		[
			SNew(SHorizontalBox) +
			SHorizontalBox::Slot().HAlign(HAlign_Left) [
				SNew(STextComboBox)
					.OptionsSource(&EventNames)
					.OnSelectionChanged(this, &FTaskEventComponentDetails::OnItemSelected)
					.InitiallySelectedItem(EventNames[SelectedEventNameID])
			]
		];
}

void FTaskEventComponentDetails::OnItemSelected(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo)
{
	if (ItemSelected.IsValid())
	{
		for (int i = 0; i < EventNames.Num(); i++)
		{
			if (EventNames[i] == ItemSelected)
			{
				SelectedEventNameID = i;
			}
		}
	}

	if (TaskEventComponent.IsValid())
		TaskEventComponent->EventName = *EventNames[SelectedEventNameID].Get();
}

TSharedRef<IDetailCustomization> FTaskEventComponentDetails::MakeInstance()
{
	return MakeShareable(new FTaskEventComponentDetails);
};
#endif