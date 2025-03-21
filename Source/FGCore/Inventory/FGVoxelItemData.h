// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGItemStack.h"
#include "Misc/FGVoxelMetadata.h"
#include "FGVoxelItemData.generated.h"

UCLASS()
class UFGVoxelItemData : public UFGItemStaticData
{
	GENERATED_BODY()
public:

	UFGVoxelItemData();

	//~ Begin Super
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const;
	//~ End Super

	UPROPERTY(EditDefaultsOnly, AssetRegistrySearchable)
	FGameplayTag VoxelTag;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UFGVoxelMetadata> VoxelMetadata;
};
