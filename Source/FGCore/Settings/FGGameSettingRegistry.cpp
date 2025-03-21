// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGGameSettingRegistry.h"
#include "FGSettingsLocal.h"
#include "FGSettingsShared.h"
#include "Player/FGLocalPlayer.h"

#include "GameSettingCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGGameSettingRegistry)

DEFINE_LOG_CATEGORY(LogFGGameSettingRegistry);

#define LOCTEXT_NAMESPACE "FG"

//--------------------------------------
// UFGGameSettingRegistry
//--------------------------------------

UFGGameSettingRegistry::UFGGameSettingRegistry()
{
}

UFGGameSettingRegistry* UFGGameSettingRegistry::Get(UFGLocalPlayer* InLocalPlayer)
{
	UFGGameSettingRegistry* Registry = FindObject<UFGGameSettingRegistry>(InLocalPlayer, TEXT("FGGameSettingRegistry"), true);
	if (Registry == nullptr)
	{
		Registry = NewObject<UFGGameSettingRegistry>(InLocalPlayer, TEXT("FGGameSettingRegistry"));
		Registry->Initialize(InLocalPlayer);
	}

	return Registry;
}

bool UFGGameSettingRegistry::IsFinishedInitializing() const
{
	if (Super::IsFinishedInitializing())
	{
		if (UFGLocalPlayer* LocalPlayer = Cast<UFGLocalPlayer>(OwningLocalPlayer))
		{
			if (LocalPlayer->GetSharedSettings() == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

void UFGGameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	UFGLocalPlayer* FGLocalPlayer = Cast<UFGLocalPlayer>(InLocalPlayer);

	VideoSettings = InitializeVideoSettings(FGLocalPlayer);
	InitializeVideoSettings_FrameRates(VideoSettings, FGLocalPlayer);
	RegisterSetting(VideoSettings);

	AudioSettings = InitializeAudioSettings(FGLocalPlayer);
	RegisterSetting(AudioSettings);

	GameplaySettings = InitializeGameplaySettings(FGLocalPlayer);
	RegisterSetting(GameplaySettings);

	MouseAndKeyboardSettings = InitializeMouseAndKeyboardSettings(FGLocalPlayer);
	RegisterSetting(MouseAndKeyboardSettings);

	GamepadSettings = InitializeGamepadSettings(FGLocalPlayer);
	RegisterSetting(GamepadSettings);
}

void UFGGameSettingRegistry::SaveChanges()
{
	Super::SaveChanges();
	
	if (UFGLocalPlayer* LocalPlayer = Cast<UFGLocalPlayer>(OwningLocalPlayer))
	{
		// Game user settings need to be applied to handle things like resolution, this saves indirectly
		LocalPlayer->GetLocalSettings()->ApplySettings(false);
		
		LocalPlayer->GetSharedSettings()->ApplySettings();
		LocalPlayer->GetSharedSettings()->SaveSettings();
	}
}

#undef LOCTEXT_NAMESPACE

