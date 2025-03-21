// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "World/FGVoxelActorManager.h"
#include "FGPowerPylon.generated.h"

/**
 * Provides a power field to nearby voxels similar to a StarCraft pylon.
 * This is primary way power is distributed in the game, you have a power
 * provider (aka nuclear, solar, ect), then you connect it via kinetic pipes
 * to pylons which distribute the power. Power is not required by a building,
 * it is ONLY a speed boost, all machines should be able to run without power.
 */
UCLASS()
class AFGPowerPylon : public AFGVoxelActor
{
	GENERATED_BODY()
public:
	
};
