// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGSteamDeckModule.h"

#include "SteamSharedModule.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "Logging/StructuredLog.h"

#include "steam/steam_api.h"

void FFGSteamDeckModule::StartupModule()
{
	// Query the Steam Deck status
	if (IsRunningOnSteamDeck())
	{
		TArray<FString> ConfigFiles { TEXT("Engine"), TEXT("Game"), TEXT("Input"), TEXT("DeviceProfiles") };
		for (const FString& ConfigFile : ConfigFiles)
		{
			const FString OverrideConfigPath = FPaths::Combine(FPaths::ProjectConfigDir(), "SteamDeck", "SteamDeck" + ConfigFile + ".ini");
		
			FConfigFile* FoundConfig = GConfig->FindConfigFile(ConfigFile);
			FoundConfig->AddDynamicLayerToHierarchy(OverrideConfigPath);

			const FString OverrideConfigPathB = FPaths::Combine(FPaths::ProjectConfigDir(), "SteamDeck", "SteamDeck" + ConfigFile + ".ini");
		}

		// Once engine initialization completes, swap our device profile.
		FCoreDelegates::OnAllModuleLoadingPhasesComplete.AddLambda([]()
		{		
			UDeviceProfileManager& DeviceProfileManager = UDeviceProfileManager::Get();
			UDeviceProfile* SteamDeckDeviceProfile = DeviceProfileManager.FindProfile("SteamDeck", false);

			if(SteamDeckDeviceProfile)
			{
				DeviceProfileManager.SetOverrideDeviceProfile(SteamDeckDeviceProfile);
				UE_LOGFMT(LogTemp, Log, "FGSteamDeckModule: Detected Steam Deck, overriding device profile.");
			}
		});
	}
}

bool FFGSteamDeckModule::IsRunningOnSteamDeck() const
{
	if (!FSteamSharedModule::IsAvailable())
	{
		// Load the module
		FSteamSharedModule::Get();
	}
	
	if (FSteamSharedModule::IsAvailable())
	{
		FSteamSharedModule& SteamSharedModule = FSteamSharedModule::Get();
        
		// Check if the Steamworks DLLs are loaded
		if (SteamSharedModule.AreSteamDllsLoaded() && SteamUtils())
		{
			return SteamUtils()->IsSteamRunningOnSteamDeck();
		}
	}

	// SteamUtils() not yet available, often due to the LoadingPhase
	return IsSteamDeck();
}

bool FFGSteamDeckModule::IsSteamDeck() const
{
	return FPlatformMisc::GetEnvironmentVariable(TEXT("SteamDeck")).Equals(FString(TEXT("1")));	
}

bool FFGSteamDeckModule::IsInDesktopMode() const
{
	const bool bSteamOS = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamOS")).Equals(FString(TEXT("1")));
	return IsSteamDeck() && bSteamOS;	
}

bool FFGSteamDeckModule::IsInBigPictureMode() const
{
	const bool bSteamOS = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamOS")).Equals(FString(TEXT("1")));
	return IsSteamDeck() && !bSteamOS;	
}

IMPLEMENT_MODULE(FFGSteamDeckModule, FGSteamDeck)
