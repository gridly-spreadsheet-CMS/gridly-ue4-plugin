// Copyright (c) 2021 LocalizeDirect AB

using UnrealBuildTool;

public class Gridly : ModuleRules
{
	public Gridly(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"InputCore",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "HTTP",
                "Serialization",
				"Json",
				"JsonUtilities"
			}
			);

		if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Settings",
                    "Localization"
                }
                );
        }
	}
}
