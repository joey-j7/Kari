#include "TableRevisioning.h"

#include "Paths.h"
#include "FileManager.h"

#include "PlatformFilemanager.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "SButton.h"
#include "SScrollBox.h"
#include "SlateApplication.h"
#include "TableDatabase.h"

#if WITH_EDITOR
	#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
#endif


TSharedRef<SWidget> STableRevOverviewRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	FString ItemText;
	
	if (ColumnName == "Name")
	{
		ItemText = *Item->Name;
	}
	else if (ColumnName == "Restore")
	{
		return SNew(SButton).Text(FText::FromString("Restore")).OnClicked_Lambda([&]()->FReply
		{
			FText confirm = FText::FromString("Confirmation");
			if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString("This will replace your current database version. Your current version will be lost, are you sure you want to continue?"), &confirm) == EAppReturnType::No)
				return FReply::Unhandled();

			FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
			path.Append("Data/");

			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			FString bakPath = path + "Backups/" + *Item->Name + "/";

			// Find backup files
			TArray<FString> bakFiles;
			PlatformFile.FindFiles(bakFiles, *bakPath, UTF8_TO_TCHAR(".udat"));

			if (bakFiles.Num() == 0)
			{
				FNotificationInfo Info(FText::FromString("Unable to restore files!"));
				Info.FadeInDuration = 0.1f;
				Info.FadeOutDuration = 0.5f;
				Info.ExpireDuration = 1.5f;
				Info.bUseThrobber = false;
				Info.bUseSuccessFailIcons = false;
				Info.bUseLargeFont = true;
				Info.bFireAndForget = true;
				Info.bAllowThrottleWhenFrameRateIsLow = false;

				auto notification = FSlateNotificationManager::Get().AddNotification(Info);
				notification->SetCompletionState(SNotificationItem::CS_Fail);

				return FReply::Unhandled();
			}

			// Delete main files
			TArray<FString> files;
			PlatformFile.FindFiles(files, *path, UTF8_TO_TCHAR(".udat"));

			for (FString& file : files)
			{
				PlatformFile.DeleteFile(*file);
			}

			PlatformFile.DeleteFile(*(path + "update.time"));

			// Copy over backup files
			FString oPath, name, ext;

			for (FString& file : bakFiles)
			{
				FPaths::Split(file, oPath, name, ext);
				PlatformFile.CopyFile(*(path + name + "." + ext), *file);
			}

			PlatformFile.CopyFile(*(path + "update.time"), *(bakPath + "update.time"));

			// Refresh all data
			TableDatabase::Get().Refresh();

			FNotificationInfo Info(FText::FromString("Backup succesfully restored"));
			Info.FadeInDuration = 0.1f;
			Info.FadeOutDuration = 0.5f;
			Info.ExpireDuration = 1.5f;
			Info.bUseThrobber = false;
			Info.bUseSuccessFailIcons = false;
			Info.bUseLargeFont = true;
			Info.bFireAndForget = true;
			Info.bAllowThrottleWhenFrameRateIsLow = false;

			auto notification = FSlateNotificationManager::Get().AddNotification(Info);
			notification->SetCompletionState(SNotificationItem::CS_Success);

			return FReply::Handled();
		});
	}
	else if (ColumnName == "Delete")
	{
		return SNew(SButton).Text(FText::FromString("Delete")).OnClicked_Lambda([&]()->FReply
		{
			FText confirm = FText::FromString("Confirmation");
			if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString("Are you sure you want to delete this backup?"), &confirm) == EAppReturnType::No)
				return FReply::Unhandled();

			FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
			path.Append("Data/");

			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

			FString bakPath = path + "Backups/" + *Item->Name;
			
			TArray<FString> files;
			PlatformFile.FindFiles(files, *bakPath, nullptr);

			for (FString& file : files)
			{
				PlatformFile.DeleteFile(*file);
			}

			if (PlatformFile.DeleteDirectory(*bakPath))
			{
				Item->Hidden = true;
				SetVisibility(EVisibility::Hidden);

				return FReply::Handled();
			}

			FNotificationInfo Info(FText::FromString("Could not delete backup!"));
			Info.FadeInDuration = 0.1f;
			Info.FadeOutDuration = 0.5f;
			Info.ExpireDuration = 1.5f;
			Info.bUseThrobber = false;
			Info.bUseSuccessFailIcons = false;
			Info.bUseLargeFont = true;
			Info.bFireAndForget = true;
			Info.bAllowThrottleWhenFrameRateIsLow = false;

			auto notification = FSlateNotificationManager::Get().AddNotification(Info);
			notification->SetCompletionState(SNotificationItem::CS_Fail);

			return FReply::Unhandled();
		});
	}

	return SNew(STextBlock).Text(FText::FromString(ItemText));
}

void STableRevOverview::Construct(const FArguments& Args)
{
	TSharedPtr<SHeaderRow> headerRow = SNew(SHeaderRow)
		+ SHeaderRow::Column("Name").DefaultLabel(FText::FromString("Name"))
		+ SHeaderRow::Column("Restore").DefaultLabel(FText::FromString("Restore"))
		+ SHeaderRow::Column("Delete").DefaultLabel(FText::FromString("Delete"));

	headerRow->SetColumnWidth("Restore", 0.1f);
	headerRow->SetColumnWidth("Delete", 0.1f);

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(ListViewWidget, SListView<TSharedPtr<TableRevItem>>)
					.ItemHeight(24)
					.ListItemsSource(&Items)
					.OnGenerateRow(this, &STableRevOverview::OnGenerateRowForList)
					.HeaderRow
					(
						headerRow
					)
				]
			]
		];

	Reset();
}

void STableRevOverview::Reset()
{
	ClearList();

	FString resPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	resPath.Append("Data/");

	FString backupPath = resPath + "Backups/";

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> Folders;

	FFolderMatch FolderMatch(Folders);
	PlatformFile.IterateDirectory(*backupPath, FolderMatch);

	for (FString& folder : Folders)
	{
		AddItem({ folder, false });
	}
}

TSharedRef<ITableRow> STableRevOverview::OnGenerateRowForList(TSharedPtr<TableRevItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRevOverviewRow, OwnerTable, Item).Visibility(Item->Hidden ? EVisibility::Hidden : EVisibility::Visible);
}

void STableRevOverview::AddItem(const TableRevItem& item)
{
	Items.Add(MakeShareable(new TableRevItem(item)));
	ListViewWidget->RequestListRefresh();
}

void STableRevOverview::ClearList()
{
	Items.Empty();
	ListViewWidget->RequestListRefresh();
}

void TableRevisioning::Backup()
{
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FString output = "Could not write the backup for the local database";
	bool succeeded = false;

	if (PlatformFile.CreateDirectory(*(path + "Backups")))
	{
		FString folderDir = path + "Backups/" + FDateTime::Now().ToString();

		if (PlatformFile.CreateDirectory(*(folderDir + '\0')))
		{
			folderDir += '/';

			TArray<FString> files;
			PlatformFile.FindFiles(files, *path, UTF8_TO_TCHAR(".udat"));

			FString oPath, name, ext;

			for (FString& file : files)
			{
				FPaths::Split(file, oPath, name, ext);
				PlatformFile.CopyFile(*(folderDir + name + "." + ext), *file);
			}

			PlatformFile.CopyFile(*(folderDir + "update.time"), *(path + "update.time"));
			output = "The backup for the local database has been succesfully made";

			succeeded = true;
		}
	}

	FNotificationInfo Info(FText::FromString(output));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 1.5f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = true;
	Info.bAllowThrottleWhenFrameRateIsLow = false;

	auto notification = FSlateNotificationManager::Get().AddNotification(Info);
	notification->SetCompletionState(succeeded ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
}

void TableRevisioning::Restore()
{
	// Find available backups
	auto tableRevOverview = SNew(STableRevOverview);
	FString oPath, name, ext;

	// Create window
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("Table Backup Overview")))
		.SizingRule(ESizingRule::UserSized)

		.ClientSize(FVector2D(800, 600))

		.FocusWhenFirstShown(true)
		.IsTopmostWindow(false)

		.SupportsMinimize(false)
		.SupportsMaximize(false)
	;

	Window->SetContent(tableRevOverview);

#if WITH_EDITOR
	IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");

	auto ParentWindow = MainFrame.GetParentWindow();
	FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);
#endif
	Window->ShowWindow();
}
