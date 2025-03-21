// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelGeneratorFlat.h"
#include "FGVoxelUtils.h"
#include "Containers/FGVoxelChunk.h"
#include "Containers/FGVoxelGrid.h"
#include "GameplayTagsManager.h"

void UFGVoxelGeneratorFlat::Generate(TArray<FFGVoxelChunk>& ChunkData, FFGChunkHandle ChunkHandle)
{
	using namespace FG::Const;

	Super::Generate(ChunkData, ChunkHandle);
	
	UGameplayTagsManager& TagMgr = UGameplayTagsManager::Get();
	FGameplayTag GrassTagName = TagMgr.RequestGameplayTag("Voxel.FG.Grass");
	int32 GrassId = GVoxelTypeMap.FindChecked(GrassTagName);

	static constexpr double TerrainHeight = -VoxelSizeUU; // Terrain height.

	FVector ChunkLocation = UFGVoxelUtils::ChunkCoordToVector(ChunkHandle->ChunkCoordinate);
	FFGVoxelChunk* VoxelChunk = GetOwningVoxelGrid()->GetChunkDataUnsafe(ChunkHandle);
	
	int32 VoxelIndex = 0;
	for(int32 VoxelX = 0; VoxelX < ChunkSizeX; VoxelX++)
	{
		for(int32 VoxelY = 0; VoxelY < ChunkSizeX; VoxelY++)
		{
			for(int32 VoxelZ = 0; VoxelZ < ChunkSizeX; VoxelZ++, VoxelIndex++)
			{
				float VoxelZHeight = ChunkLocation.Z + VoxelZ * VoxelSizeUU;
				int32 PaintType = VoxelZHeight < TerrainHeight;

				VoxelChunk->SetVoxel(VoxelIndex, GrassId * PaintType);
			}
		}
	}
}
