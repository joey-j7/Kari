// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "DatabaseSyncStyle.h"

class FDatabaseSyncCommands : public TCommands<FDatabaseSyncCommands>
{
public:

	FDatabaseSyncCommands()
		: TCommands<FDatabaseSyncCommands>(TEXT("DatabaseSync"), NSLOCTEXT("Contexts", "DatabaseSync", "DatabaseSync Plugin"), NAME_None, FDatabaseSyncStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
	TSharedPtr<FUICommandInfo> ViewTables;
	TSharedPtr<FUICommandInfo> ForceSync;
	TSharedPtr<FUICommandInfo> Login;

	TSharedPtr<FUICommandInfo > BackupAction;
	TSharedPtr<FUICommandInfo > RestoreAction;
};
