// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Engine/LocalPlayer.h"
#include "FGLocalPlayer.generated.h"

class APlayerController;
class UInputMappingContext;
class UFGSettingsLocal;
class UFGSettingsShared;
class UObject;
class UWorld;
struct FFrame;
struct FSwapAudioOutputResult;

UCLASS()
class FGCORE_API UFGLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:

	//~ Begin Super
	virtual void PostInitProperties() override;
	//~End Super

	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	UFUNCTION()
	UFGSettingsLocal* GetLocalSettings() const;

	/** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	UFUNCTION()
	UFGSettingsShared* GetSharedSettings() const;

	/** Starts an async request to load the shared settings, this will call OnSharedSettingsLoaded after loading or creating new ones */
	void LoadSharedSettingsFromDisk(bool bForceLoad = false);

protected:
	void OnSharedSettingsLoaded(UFGSettingsShared* LoadedOrCreatedSettings);

	void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);
	
	UFUNCTION()
	void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<UFGSettingsShared> SharedSettings;

	FUniqueNetIdRepl NetIdForSharedSettings;

	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
};
