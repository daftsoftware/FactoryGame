// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "Player/FGLocalPlayer.h"
#include "FGGameSettingRegistry.h"
#include "FGSettingsLocal.h"
#include "FGSettingsShared.h"

#include "CustomSettings\FGSettingValueDiscreteDynamic_AudioOutputDevice.h"
#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "GameSettingCollection.h"
#include "GameSettingValueScalarDynamic.h"
#include "NativeGameplayTags.h"

class ULocalPlayer;

#define LOCTEXT_NAMESPACE "FG"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_SupportsChangingAudioOutputDevice, "Platform.Trait.SupportsChangingAudioOutputDevice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_SupportsBackgroundAudio, "Platform.Trait.SupportsBackgroundAudio");

UGameSettingCollection* UFGGameSettingRegistry::InitializeAudioSettings(UFGLocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("AudioCollection"));
	Screen->SetDisplayName(LOCTEXT("AudioCollection_Name", "Audio"));
	Screen->Initialize(InLocalPlayer);

	// Volume
	////////////////////////////////////////////////////////////////////////////////////
	{
		UGameSettingCollection* Volume = NewObject<UGameSettingCollection>();
		Volume->SetDevName(TEXT("VolumeCollection"));
		Volume->SetDisplayName(LOCTEXT("VolumeCollection_Name", "Volume"));
		Screen->AddSetting(Volume);

		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("OverallVolume"));
			Setting->SetDisplayName(LOCTEXT("OverallVolume_Name", "Overall"));
			Setting->SetDescriptionRichText(LOCTEXT("OverallVolume_Description", "Adjusts the volume of everything."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetOverallVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetOverallVolume));
			Setting->SetDefaultValue(GetDefault<UFGSettingsLocal>()->GetOverallVolume());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Volume->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("MusicVolume"));
			Setting->SetDisplayName(LOCTEXT("MusicVolume_Name", "Music"));
			Setting->SetDescriptionRichText(LOCTEXT("MusicVolume_Description", "Adjusts the volume of music."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetMusicVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetMusicVolume));
			Setting->SetDefaultValue(GetDefault<UFGSettingsLocal>()->GetMusicVolume());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Volume->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("SoundEffectsVolume"));
			Setting->SetDisplayName(LOCTEXT("SoundEffectsVolume_Name", "Sound Effects"));
			Setting->SetDescriptionRichText(LOCTEXT("SoundEffectsVolume_Description", "Adjusts the volume of sound effects."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetSoundFXVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetSoundFXVolume));
			Setting->SetDefaultValue(GetDefault<UFGSettingsLocal>()->GetSoundFXVolume());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Volume->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("DialogueVolume"));
			Setting->SetDisplayName(LOCTEXT("DialogueVolume_Name", "Dialogue"));
			Setting->SetDescriptionRichText(LOCTEXT("DialogueVolume_Description", "Adjusts the volume of dialogue for game characters and voice overs."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDialogueVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDialogueVolume));
			Setting->SetDefaultValue(GetDefault<UFGSettingsLocal>()->GetDialogueVolume());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Volume->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("VoiceChatVolume"));
			Setting->SetDisplayName(LOCTEXT("VoiceChatVolume_Name", "Voice Chat"));
			Setting->SetDescriptionRichText(LOCTEXT("VoiceChatVolume_Description", "Adjusts the volume of voice chat."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetVoiceChatVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVoiceChatVolume));
			Setting->SetDefaultValue(GetDefault<UFGSettingsLocal>()->GetVoiceChatVolume());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Volume->AddSetting(Setting);
		}
	}


	// Sound
	////////////////////////////////////////////////////////////////////////////////////
	{
		UGameSettingCollection* Sound = NewObject<UGameSettingCollection>();
		Sound->SetDevName(TEXT("SoundCollection"));
		Sound->SetDisplayName(LOCTEXT("SoundCollection_Name", "Sound"));
		Screen->AddSetting(Sound);

		//----------------------------------------------------------------------------------
		{
			UGameSettingCollectionPage* SubtitlePage = NewObject<UGameSettingCollectionPage>();
			SubtitlePage->SetDevName(TEXT("SubtitlePage"));
			SubtitlePage->SetDisplayName(LOCTEXT("SubtitlePage_Name", "Subtitles"));
			SubtitlePage->SetDescriptionRichText(LOCTEXT("SubtitlePage_Description", "Configure the visual appearance of subtitles."));
			SubtitlePage->SetNavigationText(LOCTEXT("SubtitlePage_Navigation", "Options"));

			SubtitlePage->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Sound->AddSetting(SubtitlePage);
		}
		//----------------------------------------------------------------------------------
		{
			UFGSettingValueDiscreteDynamic_AudioOutputDevice* Setting = NewObject<UFGSettingValueDiscreteDynamic_AudioOutputDevice>();
			Setting->SetDevName(TEXT("AudioOutputDevice"));
			Setting->SetDisplayName(LOCTEXT("AudioOutputDevice_Name", "Audio Output Device"));
			Setting->SetDescriptionRichText(LOCTEXT("AudioOutputDevice_Description", "Changes the audio output device for game audio (not voice chat)."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetAudioOutputDeviceId));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetAudioOutputDeviceId));

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
			Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(
				TAG_Platform_Trait_SupportsChangingAudioOutputDevice,
				TEXT("Platform does not support changing audio output device"))
			);

			Sound->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
			Setting->SetDevName(TEXT("BackgroundAudio"));
			Setting->SetDisplayName(LOCTEXT("BackgroundAudio_Name", "Background Audio"));
			Setting->SetDescriptionRichText(LOCTEXT("BackgroundAudio_Description", "Turns game audio on/off when the game is in the background. When on, the game audio will continue to play when the game is minimized, or another window is focused."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetAllowAudioInBackgroundSetting));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetAllowAudioInBackgroundSetting));
			Setting->SetDefaultValue(GetDefault<UFGSettingsShared>()->GetAllowAudioInBackgroundSetting());

			Setting->AddEnumOption(EFGAllowBackgroundAudioSetting::Off, LOCTEXT("EFGAllowBackgroundAudioSetting_Off", "Off"));
			Setting->AddEnumOption(EFGAllowBackgroundAudioSetting::AllSounds, LOCTEXT("EFGAllowBackgroundAudioSetting_AllSounds", "All Sounds"));

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
			Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(
				TAG_Platform_Trait_SupportsBackgroundAudio,
				TEXT("Platform does not support background audio"))
			);

			Sound->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("HeadphoneMode"));
			Setting->SetDisplayName(LOCTEXT("HeadphoneMode_Name", "3D Headphones"));
			Setting->SetDescriptionRichText(LOCTEXT("HeadphoneMode_Description", "Enable binaural audio.  Provides 3D audio spatialization, so you can hear the location of sounds more precisely, including above, below, and behind you. Recommended for use with stereo headphones only."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(bDesiredHeadphoneMode));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(bDesiredHeadphoneMode));
			Setting->SetDefaultValue(GetDefault<UFGSettingsLocal>()->IsHeadphoneModeEnabled());

			Setting->AddEditCondition(MakeShared<FWhenCondition>(
				[](const ULocalPlayer*, FGameSettingEditableState& InOutEditState)
				{
					if (!GetDefault<UFGSettingsLocal>()->CanModifyHeadphoneModeEnabled())
					{
						InOutEditState.Kill(TEXT("Binaural Spatialization option cannot be modified on this platform"));
					}
				}));

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Sound->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("HDRAudioMode"));
			Setting->SetDisplayName(LOCTEXT("HDRAudioMode_Name", "High Dynamic Range Audio"));
			Setting->SetDescriptionRichText(LOCTEXT("HDRAudioMode_Description", "Enable high dynamic range audio. Changes the runtime processing chain to increase the dynamic range of the audio mixdown, appropriate for theater or more cinematic experiences."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(bUseHDRAudioMode));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetHDRAudioModeEnabled));
			Setting->SetDefaultValue(GetDefault<UFGSettingsLocal>()->IsHDRAudioModeEnabled());

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Sound->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
	}

	return Screen;
}

#undef LOCTEXT_NAMESPACE
