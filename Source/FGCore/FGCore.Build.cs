// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class FGCore : ModuleRules
{
	public FGCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// Auto include for Public / Private, since no split
        PublicIncludePaths.Add(ModuleDirectory);
        
        SetupIrisSupport(Target);
        
		// Editor Only Dependencies
        if (Target.Type == TargetType.Editor)
        {
	        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd"
			});
        }
        
		PublicDependencyModuleNames.AddRange(new string[] {
        });
		
		PrivateDependencyModuleNames.AddRange(new string[] {
            "FGCommon",
			"FGUserInterface",
            "GameplayAbilities",
            "FGVoxel",
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Chaos",
			"UMG",
			"SlateCore",
			"Slate",
			"StructUtils",
			"GameplayTags",
            "GameplayTasks",
            "DeveloperSettings",
			"Niagara",
            "ControlFlows",
            "GameSettings",
            "CommonLoadingScreen",
            "CommonInput",
            "CommonUI",
            "CommonUser",
            "AudioModulation",
            "AudioMixer",
            "ApplicationCore",
            "RHI",
            "ModularGameplay",
            "MovieScene",
            "LevelSequence",
			"NetCore",
			"GameStateSubsystem",
			"FGMovement",
            "PhysicsCore",
            "FGDebug",
			"GeometryCore",
            "GeometryFramework",
        });
	}
}
