// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/Actor.h"
#include "FGVoxelMesher.generated.h"

/*
	The base class for any voxel mesher extensions.
	
	This abstraction makes it pretty easy to test different rendering techniques
	for performance comparisons or just general testing / debugging.
*/
UCLASS(Abstract)
class FGVOXEL_API AFGVoxelMesher : public AActor
{
	GENERATED_BODY()
public:
	
	virtual void Initialize() {}
	virtual void Deinitialize() {}

	virtual void GenerateMesh(FIntVector ChunkCoordinate) {}
	virtual void ClearMesh(FIntVector ChunkCoordinate) {}
};
