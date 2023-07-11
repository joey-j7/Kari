#include "Kari.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "PropertyEditorModule.h"
#include "Events/ComponentDetails/DialogueEventComponentDetails.h"
#include "Events/ComponentDetails/DialogueOptionEventComponentDetails.h"
#include "Events/ComponentDetails/TaskEventComponentDetails.h"
#include "Events/ComponentDetails/TimeEventComponentDetails.h"
#include "Systems/Components/DialogueEventComponent.h"
#include "Systems/Components/DialogueOptionEventComponent.h"
#include "Systems/Components/TaskEventComponent.h"
#include "Systems/Components/TimeEventComponent.h"
#endif

#define LOCTEXT_NAMESPACE "FKariModule"

void  FKariModule::StartupModule ()
{
#if WITH_EDITOR
	// Register details panel
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	// TODO maybe find a way to make template classes.
	PropertyModule.RegisterCustomClassLayout(UDialogueEventComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FDialogueEventComponentDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UDialogueOptionEventComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FDialogueOptionEventComponentDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UTaskEventComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FTaskEventComponentDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UTimeEventComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FTimeEventComponentDetails::MakeInstance));
#endif
}

void FKariModule::ShutdownModule()
{}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_PRIMARY_GAME_MODULE(FKariModule, Kari, "Kari" );
