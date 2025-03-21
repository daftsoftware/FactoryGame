// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

using UnrealBuildTool;

public class FGVoxelEd : ModuleRules
{
	public FGVoxelEd(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add(ModuleDirectory);

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;

        PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				
			}
		);
	}
}
