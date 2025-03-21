// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelCulledMeshComponent.h"
#include "FGVoxelUtils.h"
#include "Containers/FGVoxelGrid.h"

void UFGVoxelCulledMeshComponent::GenerateMesh()
{
	UFGVoxelUtils::DebugDrawChunk(
		GetWorld(),
		ChunkHandle->ChunkCoordinate,
		FLinearColor::White,
		0.25f);
}

void UFGVoxelCulledMeshComponent::ClearMesh()
{
}