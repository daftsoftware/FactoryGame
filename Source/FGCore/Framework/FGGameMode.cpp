// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGGameMode.h"
#include "Player/FGPlayerPawn.h"
#include "Player/FGPlayerController.h"
#include "Player/FGPlayerState.h"
#include "FGGameState.h"
#include "..\UI\FGHUD.h"

AFGGameMode::AFGGameMode()
{
	DefaultPawnClass		= AFGPlayerPawn::StaticClass();
	PlayerControllerClass	= AFGPlayerController::StaticClass();
	PlayerStateClass		= AFGPlayerState::StaticClass();
	HUDClass				= AFGHUD::StaticClass();
	GameStateClass			= AFGGameState::StaticClass();
}
