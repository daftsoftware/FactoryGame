// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

using System.IO;
using UnrealBuildTool;

public class FastNoise2 : ModuleRules
{
	public FastNoise2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;
		
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Include"));

		PublicAdditionalLibraries.Add(LibraryPath);

		// Delay-load the library, so we can load it from the right place first
		PublicDelayLoadDLLs.Add(LibraryName + RuntimeLibExtension);

		// Ensure that the library is staged along with the executable
		RuntimeDependencies.Add(RuntimePath);

		PublicDefinitions.Add("FASTNOISE_LIBRARY_PATH=\"" + RelativeRuntimePath.Replace("\\", "\\\\") + "\"");

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Projects"
		});
	}
	
	private string ConfigName
	{
		get
		{
			return Target.Configuration == UnrealTargetConfiguration.Debug ? "Debug" : "Release";
		}
	}

	private string PlatformString
	{
		get
		{
			return Target.Platform.ToString();
		}
	}

	private string RuntimePath
	{
		get
		{
			return Path.Combine("$(PluginDir)", RelativeRuntimePath);
		}
	}

	private string RelativeRuntimePath
	{
		get
		{
			return Path.Combine("Binaries", PlatformString, LibraryName + RuntimeLibExtension);
		}
	}

	private string LibraryPath
	{
		get
		{
			if (Target.Platform.IsInGroup(UnrealPlatformGroup.Microsoft))
			{
				return Path.Combine(ModuleDirectory, PlatformString, ConfigName, LibraryName + LibraryExtension);
			}
			else
			{
				return RuntimePath;
			}
		}
	}

	private string LibraryName
	{
		get
		{
			if (Target.Configuration == UnrealTargetConfiguration.Debug)
			{
				if (Target.Platform == UnrealTargetPlatform.Mac ||
				    Target.Platform == UnrealTargetPlatform.IOS ||
				    Target.Platform == UnrealTargetPlatform.Android ||
				    Target.Platform == UnrealTargetPlatform.Linux ||
				    Target.Platform == UnrealTargetPlatform.LinuxArm64)
				{
					return "libFastNoiseD";
				}
				else if (Target.Platform.IsInGroup(UnrealPlatformGroup.Microsoft))
				{
					return "FastNoiseD";
				}
			}
			else
			{

				if (Target.Platform == UnrealTargetPlatform.Mac ||
				    Target.Platform == UnrealTargetPlatform.IOS ||
				    Target.Platform == UnrealTargetPlatform.Android ||
				    Target.Platform == UnrealTargetPlatform.Linux ||
				    Target.Platform == UnrealTargetPlatform.LinuxArm64)
				{
					return "libFastNoise";
				}
				else if (Target.Platform.IsInGroup(UnrealPlatformGroup.Microsoft))
				{
					return "FastNoise";
				}
			}

			throw new BuildException("Unsupported platform");
		}
	}

	private string LibraryExtension
	{
		get
		{
			if (Target.Platform.IsInGroup(UnrealPlatformGroup.Microsoft))
			{
				return ".lib";
			}
			else
			{
				return RuntimeLibExtension;
			}
		}
	}

	private string RuntimeLibExtension
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				return ".dylib";
			}
			else if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				return ".framework";
			}
			else if (Target.Platform == UnrealTargetPlatform.Android ||
			         Target.Platform == UnrealTargetPlatform.Linux ||
			         Target.Platform == UnrealTargetPlatform.LinuxArm64)
			{
				return ".so";
			}
			else if (Target.Platform.IsInGroup(UnrealPlatformGroup.Microsoft))
			{
				return ".dll";
			}

			throw new BuildException("Unsupported platform");
		}
	}
}