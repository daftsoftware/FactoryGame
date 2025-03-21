// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

using UnrealBuildTool;
using System.Collections.Generic;

public class FactoryGameEditorTarget : TargetRules
{
	public FactoryGameEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        // This flag currently treats the target as an engine module, which is wrong, UBT please fix it!!!
		// bWarnAboutMonolithicHeadersIncluded = true;

		ExtraModuleNames.AddRange(new string[]
		{
			"FGCommon",
			"FGCore",
			"FGUserInterface",
            "FGVoxel"
		});

        // Not required on binary engine build.

        // Compile LiveCodingConsole for any target.
        //PreBuildTargets.Add(new TargetInfo(
        //	"LiveCodingConsole",
        //	Target.Platform,
        //	UnrealTargetConfiguration.Development,
        //	Target.Architectures,
        //	Target.ProjectFile,
        //	null,
        //	Target.IntermediateEnvironment
        //));

        // Build automation.
        //PreBuildSteps.Add("$(ProjectDir)\\PreBuildSteps.bat");
        //PostBuildSteps.Add("$(ProjectDir)\\PostBuildSteps.bat");

        ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
    }
}
