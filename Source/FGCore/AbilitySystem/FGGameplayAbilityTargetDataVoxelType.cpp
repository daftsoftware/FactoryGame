// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGGameplayAbilityTargetDataVoxelType.h"

bool FGameplayAbilityTargetData_VoxelInfo::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_LocationInfo::NetSerialize(Ar, Map, bOutSuccess);

	Ar << VoxelType;

	return true;
}