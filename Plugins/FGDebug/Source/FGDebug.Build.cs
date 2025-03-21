// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

using UnrealBuildTool;

public class FGDebug : ModuleRules
{
	public FGDebug(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
            "Engine",
            "Niagara"
		});
	}
}
