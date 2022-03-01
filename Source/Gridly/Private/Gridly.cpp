// Copyright (c) 2021 LocalizeDirect AB

#include "Gridly.h"

#include "../Public/GridlyGameSettings.h"
#include "Core/Public/Modules/ModuleManager.h"

#if WITH_EDITOR
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

DEFINE_LOG_CATEGORY(LogGridly);

#define LOCTEXT_NAMESPACE "Gridly"

void FGridlyModule::StartupModule()
{
#if WITH_EDITOR
	// Register project settings

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory(
			"Gridly", LOCTEXT("RuntimeWDCategoryName", "Kontentum"), LOCTEXT("RuntimeWDCategoryDescription", "Gridly Settings"));

		ISettingsSectionPtr SettingsSection =
			SettingsModule->RegisterSettings("Project", "Plugins", "Gridly", LOCTEXT("RuntimeGeneralSettingsName", "Gridly"),
				LOCTEXT("RuntimeGeneralSettingsDescription", "Configuration for Gridly localization module"),
				GetMutableDefault<UGridlyGameSettings>());

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindStatic(&UGridlyGameSettings::OnSettingsSaved);
		}
	}
#endif
}

void FGridlyModule::ShutdownModule()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Gridly");
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGridlyModule, Gridly)
