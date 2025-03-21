// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameplayTagContainer.h"
#include "Experimental/Containers/RobinHoodHashTable.h"
#include "FGVoxelDefines.generated.h"

enum { VOXELTYPE_NONE = NULL };	// Empty voxel (air)

enum class EFGChunkFlags : uint32
{
	NoFlags,
	Generated = 1 << 0, // Chunk finished it's generation.
};
ENUM_CLASS_FLAGS(EFGChunkFlags)

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFGVoxelFlags : uint8
{
	NoFlags,
	Opaque			    = 1 << 0, // Is the voxel visible or data only?
	Invulnerable	    = 1 << 1, // Can the voxel be destroyed or replaced?
	CollisionEnabled	= 1 << 2, // Can the voxel be walked on or through?
};
ENUM_CLASS_FLAGS(EFGVoxelFlags)

extern inline int32 GRenderSizeX					= INDEX_NONE;
extern inline int32 GRenderSizeXY					= INDEX_NONE;
extern inline int32 GRenderSizeXYZ					= INDEX_NONE;
extern inline int32 GWorldHeight					= 256;
extern inline int32 GServerStreamingVolumeSizeX		= INDEX_NONE;
extern inline int32 GServerRelevancyBubbleSizeXY	= INDEX_NONE;
extern inline int32 GServerRelevancyBubbleSizeXYZ	= INDEX_NONE;

//extern inline TMap<FGameplayTag, UFGItemStaticData*>	GVoxelItemMap {};
extern FGVOXEL_API TMap<FGameplayTag, int32> GVoxelTypeMap;
extern FGVOXEL_API Experimental::TRobinHoodHashMap<int32, EFGVoxelFlags> GVoxelTypeFlagMap;

inline EFGVoxelFlags GetFlagsForVoxelType(int32 VoxelType)
{
	return *GVoxelTypeFlagMap.Find(VoxelType);
}

inline bool VoxelTypeHasAnyFlags(int32 VoxelType, EFGVoxelFlags InFlags)
{
	EFGVoxelFlags VoxelFlags = *GVoxelTypeFlagMap.Find(VoxelType);
	EFGVoxelFlags FlagComparison = VoxelFlags & InFlags;
	return FlagComparison != EFGVoxelFlags::NoFlags;
}

namespace FG::Const
{
	static constexpr double	VoxelSizeUU		= 64.0;
	static constexpr int32	ChunkSizeX		= 32;
	static constexpr int32	ChunkSizeXY		= FMath::Square(ChunkSizeX);
	static constexpr int32	ChunkSizeXYZ	= FMath::Cube(ChunkSizeX);
	static constexpr int32	RenderSizeMax	= 256;
}
