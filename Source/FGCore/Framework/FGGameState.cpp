// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGGameState.h"

AFGGameState::AFGGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFGGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}
