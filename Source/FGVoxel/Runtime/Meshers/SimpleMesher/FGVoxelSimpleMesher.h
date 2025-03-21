// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Meshers/FGVoxelMesher.h"
#include "FGVoxelSimpleMesher.generated.h"

class AFGVoxelSimpleChunkMesh;

/**
 * Simple mesh renderer.
 * Pools voxel chunk mesh actors and manages their lifetimes.
 */
UCLASS()
class AFGVoxelSimpleMesher final : public AFGVoxelMesher
{
	GENERATED_BODY()
public:

	AFGVoxelSimpleMesher();

	//~ Begin Super
	void Initialize() override;
	void Deinitialize() override;
	void GenerateMesh(FIntVector ChunkCoordinate) override;
	void ClearMesh(FIntVector ChunkCoordinate) override;
	//~ End Super

	UPROPERTY(Transient)
	TArray<TObjectPtr<AFGVoxelSimpleChunkMesh>> SimpleMeshPool;

	UPROPERTY(Transient)
	TMap<FIntVector, TObjectPtr<AFGVoxelSimpleChunkMesh>> SimpleMeshMappings;
	
	TArray<int32> SimpleMeshFreelist;
};
