// Copyright (c) 2021 LocalizeDirect AB

#include "GridlyCommands.h"

#include "GridlyStyle.h"

#define LOCTEXT_NAMESPACE "FGridlyModule"

FGridlyCommands::FGridlyCommands() :
	TCommands<FGridlyCommands>(
		TEXT("Gridly"), NSLOCTEXT("Contexts", "Gridly", "Gridly Plugin"), NAME_None, FGridlyStyle::GetStyleSetName())
{
}

void FGridlyCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Gridly", "Open project in Gridly", EUserInterfaceActionType::Button, FInputChord());
}

void FGridlyCommands::LaunchBrowser()
{
	FPlatformProcess::LaunchURL(TEXT("https://app.gridly.com"), nullptr, nullptr);
}

#undef LOCTEXT_NAMESPACE
