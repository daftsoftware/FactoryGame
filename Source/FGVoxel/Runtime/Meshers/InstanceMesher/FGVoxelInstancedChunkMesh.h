// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/Actor.h"
#include "FGVoxelInstancedChunkMesh.generated.h"

class UInstancedStaticMeshComponent;

/**
 * Instance mesh chunk actor.
 */
UCLASS()
class FGVOXEL_API AFGVoxelInstancedChunkMesh : public AActor
{
	GENERATED_BODY()
public:
	
	AFGVoxelInstancedChunkMesh();

	//~ Begin Super
	void PostInitProperties();
	//~ End Super
	
	void GenerateMesh();
	void ClearMesh();

	int32 ChunkIndex;

private:

	UPROPERTY(Transient, VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> ISM;
};
