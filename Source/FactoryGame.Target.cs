// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

using UnrealBuildTool;
using System.Collections.Generic;

public class FactoryGameTarget : TargetRules
{
	public FactoryGameTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// Security settings to catch common OOBE / RCEs
		// if these are enabled and cause errors, don't disable them - fix the code.
		// anytime you get a crash from a check related to these, it's a security bug.
		{
			// Enable array bounds checks ect.
			//bUseChecksInShipping = true;

			// Control flow guard flags
			// Guard essentially everytime there is an indirect jump checks it's going to a valid target.
			//if (Target.Configuration == UnrealTargetConfiguration.Shipping)
			//{
			//	Arguments.Add("/CETCOMPAT"); // Enable hardware shadow stacks.
			//	Arguments.Add("/guard:cf");  // Enable control flow guard.
			//
			//	// guard:xfg is faster but not supported in clang yet.
			//}
		}

		ExtraModuleNames.AddRange( new string[]
		{
			"FGCommon",
			"FGCore",
			"FGUserInterface",
			"FGVoxel"
		});

        ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
    }
}
