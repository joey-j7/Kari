#include "TableOverview.h"
#include "DatabaseSync.h"

TSharedRef<SWidget> STableOverviewRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	FString ItemText;

	if (ColumnName == "Name")
	{
		ItemText = Item.Get()->Name;
	}
	else if (ColumnName == "Last Modified")
	{
		ItemText = Item.Get()->LastModified.ToHttpDate();
	}
	else if (ColumnName == "Edit")
	{
		FString url = Item.Get()->URL;

		return SNew(SButton).Text(FText::FromString("Edit")).OnClicked_Lambda([url]()->FReply
		{
			FPlatformProcess::LaunchURL(*url, nullptr, nullptr);
			return FReply::Handled();
		});
	}
	else if (ColumnName == "Sync")
	{
		return SNew(SButton).Text(FText::FromString("Sync")).OnClicked_Lambda([&]()->FReply
		{
			FDatabaseSyncModule::Get()->RequestTableUpdate(Item.Get()->Name);
			return FReply::Handled();
		});
	}

	return SNew(STextBlock).Text(FText::FromString(ItemText));
}

void STableOverview::Construct(const FArguments& Args)
{
	TSharedPtr<SHeaderRow> headerRow = SNew(SHeaderRow)
		+ SHeaderRow::Column("Name").DefaultLabel(FText::FromString("Name"))
		+ SHeaderRow::Column("Last Modified").DefaultLabel(FText::FromString("Last Modified"))
		+ SHeaderRow::Column("Edit").DefaultLabel(FText::FromString("Edit"))
		+ SHeaderRow::Column("Sync").DefaultLabel(FText::FromString("Sync"));

	headerRow->SetColumnWidth("Last Modified", 0.33f);
	headerRow->SetColumnWidth("Edit", 0.1f);
	headerRow->SetColumnWidth("Sync", 0.1f);

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(ListViewWidget, SListView<TSharedPtr<FTableOverviewData>>)
					.ItemHeight(24)
					.ListItemsSource(&Items)
					.OnGenerateRow(this, &STableOverview::OnGenerateRowForList)
					.HeaderRow
					(
						headerRow
					)
				]
			]
	];
}

TSharedRef<ITableRow> STableOverview::OnGenerateRowForList(TSharedPtr<FTableOverviewData> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableOverviewRow, OwnerTable, Item);
}

void STableOverview::AddItem(const FTableOverviewData& item)
{
	Items.Add(MakeShareable(new FTableOverviewData(item)));
	ListViewWidget->RequestListRefresh();
}

void STableOverview::ClearList()
{
	Items.Empty();
	ListViewWidget->RequestListRefresh();
}