// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Components/PrimitiveComponent.h"
#include "FGVoxelCulledMeshComponent.generated.h"

struct FFGChunkHandleData;
using FFGChunkHandle = TSharedPtr<FFGChunkHandleData>;

UCLASS(Transient)
class UFGVoxelCulledMeshComponent final : public UPrimitiveComponent
{
	GENERATED_BODY()
public:

	void GenerateMesh();
    void ClearMesh();

	// @TODO: Makes more sense to put this on the mesher itself, for now it can stay.
	// in the future, culled mesh comp will be JUST for meshing, mesher can handle the rest.
	FFGChunkHandle ChunkHandle;
};
