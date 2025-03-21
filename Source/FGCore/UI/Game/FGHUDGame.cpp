// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGHUDGame.h"
#include "System/FGAssetManager.h"
#include "System/FGGameData.h"
#include "System/FGLocalMessage.h"
#include "UI/Game/Widgets/FGGameContentWidget.h"
#include "UI/FGUISettings.h"
#include "Widgets/FGLayoutWidget.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#define LOCTEXT_NAMESPACE "FGGame.HUD.Menu"

AFGHUDGame::AFGHUDGame()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFGHUDGame::BeginPlay()
{
	Super::BeginPlay();

	// Enable Input for UI centric Input Actions.
	EnableInput(PlayerOwner);
	SetupInputComponent(InputComponent);
}

void AFGHUDGame::SetupInputComponent(UInputComponent* HUDInputComponent)
{
	auto* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(HUDInputComponent);

	UInputAction* EscapeMenuAction = UFGAssetManager::GetAsset(UFGGameData::Get().InputActionEscapeMenu);
	EnhancedInputComponent->BindAction(EscapeMenuAction, ETriggerEvent::Started, this, &ThisClass::OpenEscapeMenu);

	UInputAction* InventoryAction = UFGAssetManager::GetAsset(UFGGameData::Get().InputActionInventory);
	EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ThisClass::OpenInventory);

	UInputAction* ChatAllAction = UFGAssetManager::GetAsset(UFGGameData::Get().InputActionChatAll);
	EnhancedInputComponent->BindAction(ChatAllAction, ETriggerEvent::Started, GameUI, &UFGGameContentWidget::OpenChatbox);

	UInputAction* ChatCommandAction = UFGAssetManager::GetAsset(UFGGameData::Get().InputActionChatCommand);
	EnhancedInputComponent->BindAction(ChatCommandAction, ETriggerEvent::Started, GameUI, &UFGGameContentWidget::OpenChatboxCmd);
}

TSoftClassPtr<UFGLayoutWidget> AFGHUDGame::GetLayoutClass() const
{
	return UFGGameData::Get().GameLayoutClass;
}

void AFGHUDGame::OpenEscapeMenu()
{
	if(!EscapeMenu || !EscapeMenu->IsActivated())
	{
		AddToMenuLayer(UFGGameData::Get().EscMenuWidgetClass.LoadSynchronous());
	}
}

void AFGHUDGame::OpenInventory()
{
	if(!InventoryMenu || !InventoryMenu->IsActivated())
	{
		AddToMenuLayer(UFGGameData::Get().InventoryWidgetClass.LoadSynchronous());
	}
}

void AFGHUDGame::OpenSettings()
{
	if(!SettingsMenu || !SettingsMenu->IsActivated())
	{
		AddToMenuLayer(UFGGameData::Get().SettingsUIClass.LoadSynchronous());
	}
}

void AFGHUDGame::ReceiveLocalMessage(TSubclassOf<UFGLocalMessage> MessageClass, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, FName MessageType, FText MessageText, UObject* OptionalObject)
{
	// Forward to the main UI widget, where it will decide to which sublayer widget to go.
	if (GameUI)
	{
		GameUI->ReceiveMessage(RelatedPlayerState_1, FGameplayTag::RequestGameplayTag(MessageType), MessageText);
	}
}

#undef LOCTEXT_NAMESPACE

