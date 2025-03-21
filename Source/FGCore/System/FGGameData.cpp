// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGGameData.h"
#include "FGAssetManager.h"
#include "InputAction.h"
#include "InputMappingContext.h"
//#include "FMODEvent.h"

const UFGGameData& UFGGameData::Get()
{
	return UFGAssetManager::Get().GetGameData();
}

UFGGameData* UFGGameData::GetMutable()
{
	// @FIXME: Gross. But valid usage and we will want to use this game data from BP.
	return const_cast<UFGGameData*>(&UFGAssetManager::Get().GetGameData());
}
