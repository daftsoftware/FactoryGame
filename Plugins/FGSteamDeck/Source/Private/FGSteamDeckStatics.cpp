// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGSteamDeckStatics.h"
#include "FGSteamDeckModule.h"

bool UFGSteamDeckStatics::IsRunningOnSteamDeck()
{
	return FFGSteamDeckModule::Get().IsRunningOnSteamDeck();
}