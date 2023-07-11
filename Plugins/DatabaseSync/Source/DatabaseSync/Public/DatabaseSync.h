// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "Framework/Commands/UICommandList.h"

#include "Runtime/Online/HTTP/Public/Http.h"
#include "SNotificationList.h"

class FToolBarBuilder;
class FMenuBuilder;

class FDatabaseSyncModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void RequestTableUpdate(const FString& table = "");

	static FDatabaseSyncModule* Get() { return Instance; }
	
private:
	void BackupClicked();
	void RestoreClicked();

	void ViewTablesClicked();
	void PluginButtonClicked();
	void ForceButtonClicked();
	void Login();

	void OnTableOverviewReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnLoginReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	static TSharedRef<SWidget> OnGenerateSyncMenuContent(TSharedPtr<FUICommandList> InCommandList);
	TSharedRef<ITableRow> OnGenerateTableListRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	static FDatabaseSyncModule* Instance;

	TSharedPtr<FUICommandList> PluginCommands;
	FHttpModule* Http;

	TSharedPtr<SNotificationItem> FetchNotification;
	TSharedPtr<SNotificationItem> OverviewFetchNotification;

	uint64_t LastTimeStamp = 0;

	FDateTime LastChecked;
	FString IdToken = FString();
	
	bool CanFetch = true;
	bool CanLogin = true;
};