// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelItemData.h"
#include "UObject/AssetRegistryTagsContext.h"

static TSoftObjectPtr<UFGAbilitySet> PlaceAbilitySet = TSoftObjectPtr<UFGAbilitySet>(
	FSoftObjectPath("/Game/Data/AbilitySets/VoxelPlacementAbilitySet.VoxelPlacementAbilitySet"));

UFGVoxelItemData::UFGVoxelItemData()
{
	AbilitySets.Add(PlaceAbilitySet.LoadSynchronous());
}

void UFGVoxelItemData::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);
	
	if(VoxelTag.IsValid())
	{
		Context.AddTag(FAssetRegistryTag("VoxelTag", VoxelTag.GetTagName().ToString(), FAssetRegistryTag::TT_Hidden));
	}
}