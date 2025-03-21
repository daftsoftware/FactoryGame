// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGCheatManager.h"

#include "Player/FGPlayerController.h"
#include "Inventory/FGItemSubsystem.h"

void UFGCheatManager::GiveItem(FName ItemID, int32 Count)
{
	if (AFGPlayerController* FGPC = Cast<AFGPlayerController>(GetOuterAPlayerController()))
	{
		if (FGPC->GetNetMode() == NM_Client)
		{
			return;
		}

		if (auto* ItemSys = GetWorld()->GetSubsystem<UFGItemSubsystem>())
		{
			ItemSys->GiveItemByName(FGPC, ItemID, Count);
		}
	}
}
