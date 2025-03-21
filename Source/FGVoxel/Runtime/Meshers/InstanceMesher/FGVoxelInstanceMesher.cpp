// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelInstanceMesher.h"
#include "FGVoxelInstancedChunkMesh.h"
#include "FGVoxelUtils.h"
#include "World/FGVoxelSystem.h"

AFGVoxelInstanceMesher::AFGVoxelInstanceMesher()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFGVoxelInstanceMesher::Initialize()
{
	Super::Initialize();

	auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();

	InstanceMeshPool.AddUninitialized(GRenderSizeXYZ);
	InstanceMeshFreelist.AddUninitialized(GRenderSizeXYZ);
	
	// Allocate greedy mesh pool.
	for(int32 Chunk = 0; Chunk < GRenderSizeXYZ; Chunk++)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |=	RF_Transient;
		SpawnParams.ObjectFlags &= ~RF_Transactional;
		
		InstanceMeshPool[Chunk] = GetWorld()->SpawnActor<AFGVoxelInstancedChunkMesh>(SpawnParams);
		InstanceMeshFreelist[Chunk] = Chunk;
	}

	// @TODO: We may want to directly pass in the chunk pointers at this point? Not sure.
	// Not going to premature optimize for now lets see how the performance is.
	VoxSys->OnRenderCoordinatesAdded.AddWeakLambda(this, [this](TArray<FIntVector> AddedCoordinates)
	{
		auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();
		
		for(FIntVector& Coordinate : AddedCoordinates)
		{
			FFGChunkHandle ChunkHandle = VoxSys->VoxelGrid->FindChunkChecked(Coordinate);
			
			const int32 NextFree = InstanceMeshFreelist.Pop();
			InstanceMeshMappings.Add(Coordinate, InstanceMeshPool[NextFree]);
			InstanceMeshPool[NextFree]->SetActorLocation(UFGVoxelUtils::ChunkCoordToVector(Coordinate));
			InstanceMeshPool[NextFree]->ChunkIndex = ChunkHandle->ChunkDataIndex;
			InstanceMeshPool[NextFree]->GenerateMesh();
		}
	});

	VoxSys->OnRenderCoordinatesRemoved.AddWeakLambda(this, [this](TArray<FIntVector> RemovedCoordinates)
	{
		TArray<int32> FreedIndices;
		FreedIndices.Reserve(RemovedCoordinates.Num());
		
		for(FIntVector& Coordinate : RemovedCoordinates)
        {
        	int32 Freed = InstanceMeshPool.Find(InstanceMeshMappings.FindChecked(Coordinate));
			checkf(Freed != INDEX_NONE, TEXT("Removed coordinate not found in mesh pool!"));

			InstanceMeshPool[Freed]->ClearMesh();
			InstanceMeshPool[Freed]->Reset();
			InstanceMeshMappings.Remove(Coordinate);
			FreedIndices.Emplace(Freed);
        }
		InstanceMeshFreelist.Append(FreedIndices);
	});
}

void AFGVoxelInstanceMesher::Deinitialize()
{
	Super::Deinitialize();

	for(AFGVoxelInstancedChunkMesh* InstanceMesh : InstanceMeshPool)
	{
		InstanceMesh->Destroy();
	}
}
