// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Engine/DataAsset.h"
#include "FGGameData.generated.h"

class UCommonUserWidget;
class UFGUISettings;
class UInputMappingContext;
class UInputAction;
class UFGLayoutWidget;
class UFGGameContentWidget;
class UFGActivatableWidget;
class UNiagaraDataChannelAsset;
class UFGInputConfig;
class UGameplayEffect;

/*
	UFGGameData.
	Non-mutable data asset that contains global game data.
*/
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Global Game Data", ShortTooltip = "Data asset containing global game data."))
class UFGGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// Returns the loaded game data.
	static const UFGGameData& Get();

	UFUNCTION(BlueprintPure, DisplayName="Get Global Game Data")
	static UFGGameData* GetMutable();
	
//////////////////////////////////////////////////////////////////////
//	GAMEPLAY
//////////////////////////////////////////////////////////////////////

	// Gameplay effect used to apply damage.  Uses SetByCaller for the damage magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// Gameplay effect used to apply healing.  Uses SetByCaller for the healing magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	// Gameplay effect used to add and remove dynamic tags.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;

//////////////////////////////////////////////////////////////////////
//	INPUT ACTIONS (ENHANCED INPUT)
//////////////////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> DesktopInputMappingContext;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UFGInputConfig> InputConfig;
	
//////////////////////////////////////////////////////////////////////
//	HUD INPUT ACTIONS (ENHANCED INPUT)
//////////////////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> InputActionEscapeMenu;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> InputActionInventory;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> InputActionChatAll;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> InputActionChatCommand;

//////////////////////////////////////////////////////////////////////
//	AUDIO
//////////////////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> BuildPlacementSFX;
	
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> BuildBlockedSFX;
	
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> BuildStartSFX;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> PlaceSFX;
	
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> DigSFX;
	
//////////////////////////////////////////////////////////////////////
//	USER INTERFACE
//////////////////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSoftClassPtr<UFGLayoutWidget> FrontEndLayoutClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSoftClassPtr<UFGLayoutWidget> GameLayoutClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSoftClassPtr<UFGUISettings> SettingsUIClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSoftClassPtr<UFGActivatableWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSoftClassPtr<UFGActivatableWidget> EscMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	int32 MaxChatMessageLength = 128;
	
//////////////////////////////////////////////////////////////////////
//	VISUAL
//////////////////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TArray<TSoftObjectPtr<UMaterialInterface>> DefaultPostProcessMaterials;
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TSoftObjectPtr<UMaterialInterface> BuildableGhostMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> RotatorRingMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> GridPlaneMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> DesatMaskMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> BuildLimitMesh;
	
//////////////////////////////////////////////////////////////////////
//	GENERAL
//////////////////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = "General")
	float InteractionDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "General")
	float InteractionTolerance = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "General")
	float GridSize = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	int32 QuickBarSize = 10;

	UPROPERTY(EditDefaultsOnly, Category = "General")
	TSoftObjectPtr<UNiagaraDataChannelAsset> ItemRendererDataChannel;
};
