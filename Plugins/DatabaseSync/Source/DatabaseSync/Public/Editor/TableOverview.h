#pragma once

#include "SlateBasics.h"

struct FTableOverviewData
{
	FString Id = "";
	FString Name = "Untitled";
	FDateTime LastModified = 0;
	FString URL = "";
};

class STableOverviewRow : public SMultiColumnTableRow< TSharedPtr<FTableOverviewData> >
{
public:
	SLATE_BEGIN_ARGS(STableOverviewRow) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FTableOverviewData> InItem)
	{
		Item = InItem;
		SMultiColumnTableRow< TSharedPtr<FTableOverviewData> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName);
	TSharedPtr<FTableOverviewData> Item;
};

class STableOverview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STableOverview) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FTableOverviewData> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void AddItem(const FTableOverviewData& item);
	void ClearList();

protected:
	TSharedPtr<SHeaderRow> Header;
	TArray<TSharedPtr<FTableOverviewData>> Items;
	TSharedPtr< SListView<TSharedPtr<FTableOverviewData>> > ListViewWidget;
};