// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelGeneratorNatural.h"
#include "FastNoise/FastNoise.h"
#include "FGVoxelUtils.h"
#include "GameplayTagsManager.h"
#include "Containers/FGVoxelGrid.h"

void UFGVoxelGeneratorNatural::Generate(TArray<FFGVoxelChunk>& ChunkData, FFGChunkHandle ChunkHandle)
{
	using namespace FG::Const;
	
	Super::Generate(ChunkData, ChunkHandle);

	UGameplayTagsManager& TagMgr = UGameplayTagsManager::Get();
	FGameplayTag GrassTagName = TagMgr.RequestGameplayTag("Voxel.FG.Grass");
	int32 GrassId = GVoxelTypeMap.FindChecked(GrassTagName);

	FVector ChunkLocation = UFGVoxelUtils::ChunkCoordToVector(ChunkHandle->ChunkCoordinate);

	static constexpr int32 Seed = 1337;
	auto SimplexNoise = FastNoise::NewFromEncodedNodeTree("CAA=", FastSIMD::Level_AVX2);
	
	TStaticArray<float, ChunkSizeXY> NoiseData;
	float* RESTRICT NoiseDataPtr = NoiseData.GetData();
	
	static constexpr float Frequency = 0.01f;
	static constexpr float TerrainHeight = 1000.f;
	
	SimplexNoise->GenUniformGrid2D(
		NoiseDataPtr,
		ChunkHandle->ChunkCoordinate.X * ChunkSizeX,
		ChunkHandle->ChunkCoordinate.Y * ChunkSizeX,
		ChunkSizeX,
		ChunkSizeX,
		Frequency,
		Seed);

	FFGVoxelChunk* VoxelChunk = GetOwningVoxelGrid()->GetChunkDataUnsafe(ChunkHandle);

	int32 VoxelIndex = 0;
	for(int32 VoxelX = 0; VoxelX < ChunkSizeX; VoxelX++)
	{
		for(int32 VoxelY = 0; VoxelY < ChunkSizeX; VoxelY++)
		{
			float GenHeight = NoiseDataPtr[VoxelX + VoxelY * ChunkSizeX] * TerrainHeight;
					
			for(int32 VoxelZ = 0; VoxelZ < ChunkSizeX; VoxelZ++, VoxelIndex++)
			{
				float VoxelZHeight = ChunkLocation.Z + VoxelZ * VoxelSizeUU;
				int32 PaintType = VoxelZHeight < GenHeight;

				VoxelChunk->SetVoxel(VoxelIndex, GrassId * PaintType);
			}
		}
	}
}