// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Meshers/FGVoxelMesher.h"
#include "FGVoxelCulledMesher.generated.h"

class UFGVoxelCulledMeshComponent;

/**
 * Culled mesh renderer.
 * Manages pooled culled mesh components, for extremely fast mesh generation.
 */
UCLASS(Transient)
class AFGVoxelCulledMesher final : public AFGVoxelMesher
{
	GENERATED_BODY()
public:

	AFGVoxelCulledMesher();

	//~ Begin Super
	void Initialize() override;
	void Deinitialize() override;
	void GenerateMesh(FIntVector ChunkCoordinate) override;
	void ClearMesh(FIntVector ChunkCoordinate) override;
	//~ End Super

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFGVoxelCulledMeshComponent>> MeshPool;

	UPROPERTY(Transient)
	TMap<FIntVector, TObjectPtr<UFGVoxelCulledMeshComponent>> MeshMappings;
	
	TArray<int32> Freelist;
};
