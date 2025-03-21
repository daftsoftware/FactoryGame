// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelKismetUtils.h"
#include "FGVoxelUtils.h"

int32 UFGVoxelKismetUtils::GetVoxelAtLocation(UObject* WorldContext, FVector Location)
{
	if(UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		return UFGVoxelUtils::GetVoxelTypeAtLocation(World, Location);
	}
	return VOXELTYPE_NONE;
}