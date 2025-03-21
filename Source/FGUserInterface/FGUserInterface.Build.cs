// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

using UnrealBuildTool;

public class FGUserInterface : ModuleRules
{
	public FGUserInterface(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add(ModuleDirectory);

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;

        PublicDependencyModuleNames.AddRange(new string[]
        {
		});

        PrivateDependencyModuleNames.AddRange(new string[]
	    {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"CommonInput",
			"EnhancedInput",
		    "CommonUI",
			"Slate",
			"SlateCore",
			"UMG",
		    "GameplayTags"
	    });
	}
}
