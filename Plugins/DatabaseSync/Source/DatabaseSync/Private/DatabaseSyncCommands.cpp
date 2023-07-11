// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DatabaseSyncCommands.h"

#define LOCTEXT_NAMESPACE "FDatabaseSyncModule"

void FDatabaseSyncCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Sync", "Retrieve and sync game data from the database", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(ViewTables, "Show Table Overview", "List all the available tables and have the option to sync them individually", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(ForceSync, "Force Sync", "Force syncs all available tables", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(Login, "Login", "Login in to the database", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(BackupAction, "Backup Database", "Backup the current locally stored database", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(RestoreAction, "Restore Database", "Restore the local database from a previously made backup", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
