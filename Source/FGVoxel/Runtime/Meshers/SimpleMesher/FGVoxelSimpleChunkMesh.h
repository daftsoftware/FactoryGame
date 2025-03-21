// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "GameFramework/Actor.h"
#include "Containers/FGVoxelGrid.h"
#include "FGVoxelSimpleChunkMesh.generated.h"

class UDynamicMeshComponent;

using namespace UE::Geometry;

enum class EFGVoxelMeshLOD : uint8
{
	LOD0 = 1,
	LOD1 = 2,
	LOD2 = 4,
	LOD3 = 8,
};

/**
 * Simple mesh chunk actor.
 */
UCLASS()
class FGVOXEL_API AFGVoxelSimpleChunkMesh final : public AActor
{
	GENERATED_BODY()
public:
	
	AFGVoxelSimpleChunkMesh();

	//~ Begin Super
	void PostInitializeComponents() override;
	//~ End Super
	
	void GenerateMesh();
	void ClearMesh();

	FFGChunkHandle ChunkHandle;

	UPROPERTY(Transient, VisibleAnywhere)
	TObjectPtr<UDynamicMeshComponent> DynMeshComponent;

	EFGVoxelMeshLOD MeshLOD;
	FDynamicMeshUVOverlay* UVOverlay;
	FDynamicMeshNormalOverlay* NormalOverlay;
	
private:

	void MakeQuad(FDynamicMesh3& DynMesh, FIntVector VoxelCoordinate, int32 DOF);

	int32 VertexCount;
};
