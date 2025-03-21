// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGActivatableWidget.h"

#include "CommonUITypes.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"

void UFGActivatableWidget::NativeConstruct()
{
	if(BackHandlerOverride) // Run custom back action setup.
	{
		UCommonUserWidget::NativeConstruct(); // Explicitly skip activatable widget construct.

		if (bIsBackHandler)
		{
			checkf(CommonUI::IsEnhancedInputSupportEnabled(), TEXT("Please enable enhanced input common ui support."));
			//checkf(ICommonInputModule::GetSettings().GetEnhancedInputBackAction(), TEXT("Please enable enhanced back actions."));
		
			FBindUIActionArgs BindArgs = FBindUIActionArgs(BackHandlerOverride, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleBackAction));
			BindArgs.bDisplayInActionBar = bIsBackActionDisplayedInActionBar;

			CustomBackHandler = RegisterUIActionBinding(BindArgs);
		}
	}
	else // Run default back action setup.
	{
		Super::NativeConstruct();
	}
}

void UFGActivatableWidget::NativeDestruct()
{
	if(BackHandlerOverride)
	{
		if (UGameInstance* GameInstance = GetGameInstance<UGameInstance>())
		{
			// Deactivations might rely on members of the game instance to validly run.
			// If there's no game instance, any cleanup done in Deactivation will be irrelevant; we're shutting down the game
			DeactivateWidget();
		}
		Super::NativeDestruct();

		if (CustomBackHandler.IsValid())
		{
			CustomBackHandler.Unregister();
		}	
	}
	else
	{
		Super::NativeDestruct();
	}
}

// Actually sane input handling patch - taken from Lyra.
TOptional<FUIInputConfig> UFGActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EFGWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EFGWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EFGWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EFGWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}
