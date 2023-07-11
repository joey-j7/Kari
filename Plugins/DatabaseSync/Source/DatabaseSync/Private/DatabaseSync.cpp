// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DatabaseSync.h"
#include "DatabaseSyncStyle.h"
#include "DatabaseSyncCommands.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "JsonSerializer.h"
#include "TableBinary.h"

#include <ctime>

#include "NotificationManager.h"


#include "TableDatabase.h"
#include "Kismet/GameplayStatics.h"
#include "TableOverview.h"
#include "TableRevisioning.h"

#if WITH_EDITOR 
	#include "DatabaseIDPinGraphFactory.h"
	#include "LevelEditor.h"
	#include "EditorStyle.h"
	#include "PropertyEditorModule.h"

	#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
	#include "TableSelector.h"
#endif 


static const FName DatabaseSyncTabName("DatabaseSync");
static const FName DatabaseURL("https://storytime.vamidicreations.nl/core/api/");

const char* DatabaseEmail = "designers@buas.nl";
const char* DatabasePassword = "nX&3nv~#5();4rSP";

#define LOCTEXT_NAMESPACE "FDatabaseSyncModule"

FDatabaseSyncModule* FDatabaseSyncModule::Instance = nullptr;

void FDatabaseSyncModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	Instance = this;

	FDatabaseSyncStyle::Initialize();
	FDatabaseSyncStyle::ReloadTextures();

	FDatabaseSyncCommands::Register();

	// Load
	TableDatabase::Get();

	PluginCommands = MakeShareable(new FUICommandList);

	// Map button actions
	PluginCommands->MapAction(
		FDatabaseSyncCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FDatabaseSyncModule::PluginButtonClicked),
		FCanExecuteAction()
	);

	PluginCommands->MapAction(
		FDatabaseSyncCommands::Get().ForceSync,
		FExecuteAction::CreateRaw(this, &FDatabaseSyncModule::ForceButtonClicked),
		FCanExecuteAction()
	);

	PluginCommands->MapAction(
		FDatabaseSyncCommands::Get().Login,
		FExecuteAction::CreateRaw(this, &FDatabaseSyncModule::Login),
		FCanExecuteAction()
	);

	PluginCommands->MapAction(
		FDatabaseSyncCommands::Get().ViewTables,
		FExecuteAction::CreateRaw(this, &FDatabaseSyncModule::ViewTablesClicked),
		FCanExecuteAction()
	);

	PluginCommands->MapAction(
		FDatabaseSyncCommands::Get().BackupAction,
		FExecuteAction::CreateRaw(this, &FDatabaseSyncModule::BackupClicked),
		FCanExecuteAction()
	);

	PluginCommands->MapAction(
		FDatabaseSyncCommands::Get().RestoreAction,
		FExecuteAction::CreateRaw(this, &FDatabaseSyncModule::RestoreClicked),
		FCanExecuteAction()
	);

	Http = &FHttpModule::Get();

#if WITH_EDITOR 
	// Register custom pin
	TSharedPtr<DatabaseIDPinGraphFactory> PinGraphFactory = MakeShareable(new DatabaseIDPinGraphFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(PinGraphFactory);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FDatabaseSyncModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Compile", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FDatabaseSyncModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	// Register detail customizations
	{
		auto& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >("PropertyEditor");

		// Register our customization to be used by a class 'UMyClass' or 'AMyClass'. Note the prefix must be dropped.
		PropertyModule.RegisterCustomClassLayout(
			"TableComponent",
			FOnGetDetailCustomizationInstance::CreateStatic(&FTableSelector::MakeInstance)
		);

		PropertyModule.NotifyCustomizationModuleChanged();
	}
#endif
	
	// Check if any version is present
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/update.time");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*path);

	if (!FileHandle)
	{
		RequestTableUpdate();
	}
	else
	{
		delete FileHandle;
	}
}

void FDatabaseSyncModule::ShutdownModule()
{
	// Unload
	delete &TableDatabase::Get();

#if WITH_EDITOR 
	// Unregister
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("TableComponent");
	}
#endif

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FDatabaseSyncStyle::Shutdown();

	FDatabaseSyncCommands::Unregister();
}

void FDatabaseSyncModule::RequestTableUpdate(const FString& table)
{
	if (!CanFetch || FDateTime::Now() - LastChecked < 50000000)
	{
		FText title_text = FText::FromString("Please wait");

		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString("Please don't spam this button pleb, have a little patience."),
			&title_text
		);

		return;
	}

	CanFetch = false;
	LastChecked = FDateTime::Now();

	FNotificationInfo Info(LOCTEXT("FetchDatabase", "Fetching database..."));
	// Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 1.5f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;

	FetchNotification = FSlateNotificationManager::Get().AddNotification(Info);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &FDatabaseSyncModule::OnResponseReceived);

	// Get latest check timestamp from file
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/update.time");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*path);

	// When file can be opened
	if (FileHandle)
	{
		FileHandle->Read((uint8_t*)&LastTimeStamp, sizeof(uint64_t));
		delete FileHandle;
	}
	else
	{
		LastTimeStamp = 0;
	}

	//This is the url on which to process the request
	const FString page = table.IsEmpty() ? "tables/?tstamp=" : "tables/" + table + "?tstamp=";

	Request->SetURL(DatabaseURL.ToString() + "firebase/projects/-MFFSn0ysMX3c6xD9zZI/" + page + FString::FromInt(LastTimeStamp));
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader("Authorization", "Bearer " + IdToken);
	Request->ProcessRequest();
}

void FDatabaseSyncModule::PluginButtonClicked()
{
	RequestTableUpdate();
}

void FDatabaseSyncModule::ForceButtonClicked()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/");

	// Delete main files
	TArray<FString> files;
	PlatformFile.FindFiles(files, *path, UTF8_TO_TCHAR(".udat"));

	for (FString& file : files)
	{
		PlatformFile.DeleteFile(*file);
	}

	PlatformFile.DeleteFile(*(path + "update.time"));

	RequestTableUpdate();
}

void FDatabaseSyncModule::Login()
{
	if (!CanLogin)
	{
		FText title_text = FText::FromString("Please wait");

		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString("Is still login in. A moment please!"),
			&title_text
		);

		return;
	}

	CanLogin = false;
	
	FNotificationInfo Info(LOCTEXT("LoginDatabase", "Login to database..."));
	// Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 1.5f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;

	FetchNotification = FSlateNotificationManager::Get().AddNotification(Info);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &FDatabaseSyncModule::OnLoginReceived);
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("email"), *FString::Printf(TEXT("%hs"), DatabaseEmail));
    JsonObject->SetStringField(TEXT("password"), *FString::Printf(TEXT("%hs"), DatabasePassword));

	FString OutputString;
	const TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	//This is the url on which to process the request
	Request->SetURL(DatabaseURL.ToString() + "authenticate");
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetContentAsString(OutputString);
	Request->ProcessRequest();
}

void FDatabaseSyncModule::ViewTablesClicked()
{
	FNotificationInfo Info(LOCTEXT("FetchDatabaseOverview", "Fetching database overview..."));
	// Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 1.5f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;

	OverviewFetchNotification = FSlateNotificationManager::Get().AddNotification(Info);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &FDatabaseSyncModule::OnTableOverviewReceived);

	//This is the url on which to process the request
	Request->SetURL(DatabaseURL.ToString() + "firebase/projects/-MFFSn0ysMX3c6xD9zZI/tables/");
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader("Authorization", "Bearer " + IdToken);
	Request->ProcessRequest();
}

void FDatabaseSyncModule::BackupClicked()
{
	TableRevisioning().Backup();
}

void FDatabaseSyncModule::RestoreClicked()
{
	TableRevisioning().Restore();
}

void FDatabaseSyncModule::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FetchNotification->SetCompletionState(bWasSuccessful ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	FetchNotification->ExpireAndFadeout();
	
	if (!bWasSuccessful)
	{
		CanFetch = true;
		return;
	}

	if(Response->GetResponseCode() == 401)
	{
		FNotificationInfo Info(FText::FromString("Database error:\n\" Could not retrieve data from database!"));
		
		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 5.0f;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = false;
		Info.bUseLargeFont = false;
		Info.bFireAndForget = true;
		Info.bAllowThrottleWhenFrameRateIsLow = false;

		auto Notification = FSlateNotificationManager::Get().AddNotification(Info);
		Notification->SetCompletionState(SNotificationItem::CS_Fail);

		CanFetch = true;
		return;
	}

	// Handle result
	FString string = Response->GetContentAsString();

	//Create a pointer to hold the json serialized data
	TSharedPtr<FJsonValue> JsonObject;

	//Create a reader pointer to read the json data
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(string);

	//Deserialize the json data given Reader and the actual object to deserialize
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{

		auto arr = JsonObject->AsArray();

		for (auto it = arr.begin(); it != arr.end(); ++it)
		{
			auto table = (*it)->AsObject();
			
			// key of the table
			FString key;
			TSharedPtr<FJsonObject> dataObj;
		
			// For each table
			for (auto prop = table->Values.begin(); prop != table->Values.end(); ++prop)
			{
				if(prop->Key == "data")
				{
					dataObj = prop->Value.Get()->AsObject();
				}

				if(prop->Key == "metadata")
				{
					auto metaData = prop->Value.Get()->AsObject();
					for (auto metadata = metaData->Values.begin(); metadata != metaData->Values.end(); ++metadata)
					{
						if(metadata->Key == "title")
						{
							key = metadata->Value.Get()->AsString();
							break;
						}
					}	
				}
			}
			
			// Get table name and store it as individual data
			TableBinary& bin = TableDatabase::Get().GetBinary(key);
			bin.Import(dataObj);
			bin.Export();
		}
	}

	// Clear data cache
	TableDatabase::Get().RemoveCache();

#if WITH_EDITOR 
	// Update editor
	auto& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >("PropertyEditor");
	PropertyModule.NotifyCustomizationModuleChanged();
#endif

	
	// Update timestamp
	FString path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
	path.Append("Data/update.time");

	LastTimeStamp = std::time(0);
	uint8 c[8];

	memcpy(&c[0], &LastTimeStamp, 8);

	FFileHelper::SaveArrayToFile(c, *path);
	CanFetch = true;
}

void FDatabaseSyncModule::OnTableOverviewReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	OverviewFetchNotification->SetCompletionState(bWasSuccessful ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	OverviewFetchNotification->ExpireAndFadeout();

	if (!bWasSuccessful)
	{
		FNotificationInfo Info(FText::FromString("Database: Cannot parse JSON data"));

		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 5.0f;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = false;
		Info.bUseLargeFont = false;
		Info.bFireAndForget = true;
		Info.bAllowThrottleWhenFrameRateIsLow = false;

		auto Notification = FSlateNotificationManager::Get().AddNotification(Info);
		Notification->SetCompletionState(SNotificationItem::CS_Fail);

		return;
	}

	// Handle result
	FString string = Response->GetContentAsString();

	//Create a pointer to hold the json serialized data
	TSharedPtr<FJsonValue> JsonObject;

	//Create a reader pointer to read the json data
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(string);
 
	//Deserialize the json data given Reader and the actual object to deserialize
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		auto arr = JsonObject->AsArray();

		auto tableOverview = SNew(STableOverview);

		for (auto it = arr.begin(); it != arr.end(); ++it)
		{
			auto tableObj = (*it)->AsObject();

			FTableOverviewData data;
			for (auto table = tableObj->Values.begin(); table != tableObj->Values.end(); ++table)
			{	
				// extract metadata from table object.
				if(table->Key == "id")
				{
					data.Id = table->Value.Get()->AsString();
					data.URL = DatabaseURL.ToString() + "firebase/projects/-MFFSn0ysMX3c6xD9zZI/tables/" + data.Id;
				}
				
				if(table->Key == "metadata")
				{
					auto metaData = table->Value.Get()->AsObject();
					for (auto metadata = metaData->Values.begin(); metadata != metaData->Values.end(); ++metadata)
					{
						if(metadata->Key == "title")
						{
							data.Name = metadata->Value.Get()->AsString();
							break;
						}

						int64 timestamp = static_cast<int64>((*it)->AsNumber());
						data.LastModified = FDateTime::FromUnixTimestamp(timestamp);
					}	
				}
			}

			tableOverview->AddItem(data);	
			
		}

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(FText::FromString(TEXT("Table Overview")))
			.SizingRule(ESizingRule::UserSized)

			.ClientSize(FVector2D(800, 600))

			.FocusWhenFirstShown(true)
			.IsTopmostWindow(false)
		
			.SupportsMinimize(false)
			.SupportsMaximize(false)
		;

		Window->SetContent(tableOverview);

#if WITH_EDITOR 
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");

		auto ParentWindow = MainFrame.GetParentWindow();
		FSlateApplication::Get().AddModalWindow(Window, ParentWindow, true);
#endif
		
		Window->ShowWindow();
	}
	else
	{
		FNotificationInfo Info(FText::FromString("Database: Cannot parse JSON data"));

		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 5.0f;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = false;
		Info.bUseLargeFont = false;
		Info.bFireAndForget = true;
		Info.bAllowThrottleWhenFrameRateIsLow = false;

		auto Notification = FSlateNotificationManager::Get().AddNotification(Info);
		Notification->SetCompletionState(SNotificationItem::CS_Fail);
	}
}

void FDatabaseSyncModule::OnLoginReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FetchNotification->SetCompletionState(bWasSuccessful ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	FetchNotification->ExpireAndFadeout();
	
	if (!bWasSuccessful || Response->GetResponseCode() == 401)
	{
		FNotificationInfo Info(FText::FromString("Database: Cannot login into Firebase" + Response->GetContentAsString()));
		
		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 5.0f;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = false;
		Info.bUseLargeFont = false;
		Info.bFireAndForget = true;
		Info.bAllowThrottleWhenFrameRateIsLow = false;

		auto Notification = FSlateNotificationManager::Get().AddNotification(Info);
		Notification->SetCompletionState(SNotificationItem::CS_Fail);

		CanLogin = true;
		return;
	}

	// Handle result
	FString string = Response->GetContentAsString();

	// Create a pointer to hold the json serialized data
	TSharedPtr<FJsonObject> JsonObject;

	// Create a reader pointer to read the json data
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(string);

	// Deserialize the json data given Reader and the actual object to deserialize
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		// For each table
		for (auto token = JsonObject->Values.begin(); token != JsonObject->Values.end(); ++token)
		{
			std::string key = TCHAR_TO_UTF8(*token->Key);
			if (key == "id_token")
			{
				FNotificationInfo Info(FText::FromString("Login successful, parsing token"));
				
				Info.FadeInDuration = 0.1f;
				Info.FadeOutDuration = 0.5f;
				Info.ExpireDuration = 5.0f;
				Info.bUseThrobber = false;
				Info.bUseSuccessFailIcons = false;
				Info.bUseLargeFont = false;
				Info.bFireAndForget = true;
				Info.bAllowThrottleWhenFrameRateIsLow = false;

				auto Notification = FSlateNotificationManager::Get().AddNotification(Info);
				Notification->SetCompletionState(SNotificationItem::CS_Success);

				IdToken = token->Value.Get()->AsString();
			}
		}
	}

	CanLogin = true;
}

TSharedRef<ITableRow> FDatabaseSyncModule::OnGenerateTableListRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	//Create the row
	return
		SNew(STableRow< TSharedPtr<FString> >, OwnerTable)
		.Padding(2.0f)
		[
			SNew(STextBlock).Text(FText::FromString(*Item.Get()))
		]
	;
}

void FDatabaseSyncModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FDatabaseSyncCommands::Get().PluginAction);
}

TSharedRef<SWidget> FDatabaseSyncModule::OnGenerateSyncMenuContent(TSharedPtr<FUICommandList> InCommandList)
{
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, InCommandList);

	MenuBuilder.BeginSection("SyncTables", LOCTEXT("SyncTables", "Actions"));
	{
		MenuBuilder.AddMenuEntry(FDatabaseSyncCommands::Get().ViewTables);
		MenuBuilder.AddMenuEntry(FDatabaseSyncCommands::Get().ForceSync);
		MenuBuilder.AddMenuEntry(FDatabaseSyncCommands::Get().Login);
	}

	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("RevisionActions", LOCTEXT("RevisionActions", "Revisioning"));
	{
		MenuBuilder.AddMenuEntry(FDatabaseSyncCommands::Get().BackupAction);
		MenuBuilder.AddMenuEntry(FDatabaseSyncCommands::Get().RestoreAction);
	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FDatabaseSyncModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
#if WITH_EDITOR 
	Builder.AddToolBarButton(FDatabaseSyncCommands::Get().PluginAction);

	Builder.AddComboButton(
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction(),
			FIsActionChecked(),
			FIsActionButtonVisible()
		),
		FOnGetContent::CreateStatic(&FDatabaseSyncModule::OnGenerateSyncMenuContent, PluginCommands),
		LOCTEXT("SyncCombo_Label", "Sync Options"),
		LOCTEXT("SyncToolTip", "Sync options menu"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "DatabaseSync.PluginAction"),
		true
	);
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDatabaseSyncModule, DatabaseSync)