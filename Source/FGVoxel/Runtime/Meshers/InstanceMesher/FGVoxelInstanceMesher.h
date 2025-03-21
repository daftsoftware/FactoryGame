// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Meshers/FGVoxelMesher.h"
#include "FGVoxelInstanceMesher.generated.h"

class AFGVoxelInstancedChunkMesh;

/**
 * Instance mesh renderer.
 * Pools voxel chunk mesh actors and manages their lifetimes.
 */
UCLASS()
class AFGVoxelInstanceMesher final : public AFGVoxelMesher
{
	GENERATED_BODY()
public:

	AFGVoxelInstanceMesher();

	//~ Begin Super
	void Initialize() override;
	void Deinitialize() override;
	//~ End Super

	UPROPERTY(Transient)
	TArray<TObjectPtr<AFGVoxelInstancedChunkMesh>> InstanceMeshPool;

	TArray<int32> InstanceMeshFreelist;
	TMap<FIntVector, AFGVoxelInstancedChunkMesh*> InstanceMeshMappings;
};
