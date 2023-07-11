#pragma once
#include "SlateBasics.h"
#include "SGraphPin.h"

class SDatabaseIDPinGraph : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SDatabaseIDPinGraph) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;

	void OnTableSelected(TSharedPtr<FString> Selected, ESelectInfo::Type SelectInfo);
	void OnColumnSelected(TSharedPtr<FString> Selected, ESelectInfo::Type SelectInfo);
	void OnEntrySelected(TSharedPtr<FString> Selected, ESelectInfo::Type SelectInfo);

private:
	void CreateTableDropdownValues();
	void CreateColumnDropdownValues();
	void CreateEntryDropdownValues();

	void SetValue();

	TArray<TSharedPtr<FString>> TableNames;
	int32 TableID = 0;
	TSharedPtr<STextComboBox> TableWidget;

	TArray<TSharedPtr<FString>> ColumnNames;
	int32 ColumnID = 0;
	TSharedPtr<STextComboBox> ColumnWidget;

	TArray<TSharedPtr<FString>> EntryNames;
	TMap<int32, int32> EntryIDMapping;
	int32 EntryID = 0;
	TSharedPtr<STextComboBox> EntryWidget;
};