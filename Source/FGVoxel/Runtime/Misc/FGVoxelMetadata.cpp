// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelMetadata.h"
#include "Containers/FGVoxelChunk.h"

UFGVoxelMetadata::UFGVoxelMetadata()
	: Flags(NULL)
{
	Flags |= (int32)EFGVoxelFlags::Opaque;
}

FPrimaryAssetId UFGVoxelMetadata::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("VoxelMetadata"), *VoxelName.ToString());
}