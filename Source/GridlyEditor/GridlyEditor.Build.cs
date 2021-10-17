// Copyright (c) 2021 LocalizeDirect AB

using UnrealBuildTool;

public class GridlyEditor : ModuleRules
{
	public GridlyEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"LocalizationService",
				"Json",
				"JsonUtilities",
				"HTTP",
				"Serialization",
				"Localization",
				"LocalizationCommandletExecution",
				"MainFrame",
				"DesktopPlatform",
				"Gridly"
			}
		);
	}
}