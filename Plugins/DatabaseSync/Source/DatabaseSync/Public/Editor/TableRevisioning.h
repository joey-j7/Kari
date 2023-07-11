#pragma once

#include "CoreMinimal.h"
#include "STableViewBase.h"
#include "SListView.h"

#include "GenericPlatform/GenericPlatformFile.h"

// Class used for the visitor in iterate directory
class FFolderMatch : public IPlatformFile::FDirectoryVisitor
{
public:
	TArray<FString>& Result;

	FFolderMatch(TArray<FString>& InResult)
		: Result(InResult)
	{
	}

	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
	{
		if (bIsDirectory)
			Result.Add(FPaths::GetCleanFilename(FilenameOrDirectory));

		return bIsDirectory;
	}
};

struct TableRevItem
{
	FString Name;
	bool Hidden = false;
};

class STableRevOverview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STableRevOverview) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<TableRevItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void AddItem(const TableRevItem& item);
	void ClearList();

	void Reset();

protected:
	TSharedPtr<SHeaderRow> Header;
	TArray<TSharedPtr<TableRevItem>> Items;
	TSharedPtr< SListView<TSharedPtr<TableRevItem>> > ListViewWidget;
};

class STableRevOverviewRow : public SMultiColumnTableRow< TSharedPtr<FString> >
{
public:
	SLATE_BEGIN_ARGS(STableRevOverviewRow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<TableRevItem> InItem)
	{
		Item = InItem;
		SMultiColumnTableRow< TSharedPtr<FString> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName);
	TSharedPtr<TableRevItem> Item;

	void SetInfo(STableRevOverview& table)
	{
		Table = MakeShareable(&table);
	}

protected:
	TSharedPtr<STableRevOverview> Table;
};

class TableRevisioning
{
public:
	TableRevisioning() = default;

	void Backup();
	void Restore();
};