// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "GridlyLocalizationServiceProvider.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGridlyEditor, Log, Log);

class FGridlyEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;

private:
	FGridlyLocalizationServiceProvider GridlyLocalizationServiceProvider;
};
