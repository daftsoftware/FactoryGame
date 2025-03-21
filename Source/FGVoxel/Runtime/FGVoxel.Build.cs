// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

using System.IO;
using UnrealBuildTool;

public class FGVoxel : ModuleRules
{
	public FGVoxel(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add(ModuleDirectory);

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;
		
		//PrivateIncludePaths.AddRange(new string[]
		//{
		//	Path.Combine(GetModuleDirectory("IrisCore"), "Private"),
		//});
		
        PublicDependencyModuleNames.AddRange(new string[]
        {
		});

        PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"RenderCore",
			"RHI",
			"Niagara",
			"ProceduralMeshComponent",
			"DeveloperSettings",
			"FGCommon",
			"FastNoise2",
			"ModularGameplay",
			"Iris",
			"IrisCore",
			"NetCore",
			"GameStateSubsystem",
			"FGDebug",
			"GeometryCore",
			"GeometryFramework",
			"GameplayAbilities",
			"GameplayTags",
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd"
			});
		}
		
		SetupIrisSupport(Target);
	}
}
