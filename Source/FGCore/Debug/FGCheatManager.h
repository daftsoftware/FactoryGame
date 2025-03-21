// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/CheatManager.h"
#include "FGCheatManager.generated.h"

#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif // #ifndef USING_CHEAT_MANAGER

UCLASS(config = Game, Within = PlayerController)
class FGCORE_API UFGCheatManager : public UCheatManager
{
	GENERATED_BODY()
public:
	
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void GiveItem(FName ItemID, int32 Count);

};
