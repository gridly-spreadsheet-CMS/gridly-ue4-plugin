// Copyright © 2020 LocalizeDirect AB

#include "GridlyEditor.h"

#include "GridlyCommands.h"
#include "GridlyLocalizationServiceProvider.h"
#include "GridlyStyle.h"
#include "Json.h"
#include "ToolMenus.h"
#include "Runtime/Online/HTTP/Public/HttpModule.h"

struct FToolMenuEntry;
DEFINE_LOG_CATEGORY(LogGridlyEditor)

#define LOCTEXT_NAMESPACE "Gridly"

void FGridlyEditorModule::StartupModule()
{
	// Register localizations service

	IModularFeatures::Get().RegisterModularFeature("LocalizationService", &GridlyLocalizationServiceProvider);

	// Register button/menu item

	FGridlyStyle::Initialize();
	FGridlyStyle::ReloadTextures();
	FGridlyCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(FGridlyCommands::Get().PluginAction,
		FExecuteAction::CreateStatic(&FGridlyCommands::LaunchBrowser), FCanExecuteAction());
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGridlyEditorModule::RegisterMenus));
}

void FGridlyEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FGridlyStyle::Shutdown();
	FGridlyCommands::Unregister();

	IModularFeatures::Get().UnregisterModularFeature("LocalizationService", &GridlyLocalizationServiceProvider);
}

void FGridlyEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
	{
		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FGridlyCommands::Get().PluginAction));
		Entry.SetCommandList(PluginCommands);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGridlyEditorModule, GridlyEditor);
