// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelSimpleMesher.h"
#include "FGVoxelSimpleChunkMesh.h"
#include "FGVoxelUtils.h"
#include "Logging/StructuredLog.h"
#include "World/FGVoxelSystem.h"

namespace FG
{
	static FAutoConsoleCommandWithWorld CmdDumpSimpleMesherChunks(
		TEXT("FG.DumpSimpleMesherChunks"),
		TEXT("Dump chunk coordinates from the simple mesher."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			auto* VoxSys = World->GetSubsystem<UFGVoxelSystem>();
			auto* Mesher = CastChecked<AFGVoxelSimpleMesher>(VoxSys->ActiveMesher.GetValue());

			for(auto Mapping : Mesher->SimpleMeshMappings)
			{
				UFGVoxelUtils::DebugDrawChunk(World, Mapping.Key, FLinearColor::White);
				UE_LOGFMT(LogTemp, Warning, "{Coordinate}", Mapping.Key.ToString());
			}
		})
	);

	static FAutoConsoleCommandWithWorld CmdMarkSimpleMesherDirty(
		TEXT("FG.MarkSimpleMesherDirty"),
		TEXT("Mark render state dirty for all simple mesher chunks."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			auto* VoxSys = World->GetSubsystem<UFGVoxelSystem>();
			auto* Mesher = CastChecked<AFGVoxelSimpleMesher>(VoxSys->ActiveMesher.GetValue());

			for(auto Mapping : Mesher->SimpleMeshMappings)
			{
				//Mapping.Value->DynMeshComponent->MarkRenderStateDirty();
			}
			
			UE_LOGFMT(LogTemp, Warning, "Marking render state of {Num} chunks dirty.", Mesher->SimpleMeshMappings.Num());
		})
	);
}

AFGVoxelSimpleMesher::AFGVoxelSimpleMesher()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFGVoxelSimpleMesher::Initialize()
{
	Super::Initialize();

	auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();

	SimpleMeshPool.AddUninitialized(GRenderSizeXYZ);
	SimpleMeshFreelist.AddUninitialized(GRenderSizeXYZ);
	
	for(int32 Chunk = 0; Chunk < GRenderSizeXYZ; Chunk++)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |=	RF_Transient;
		SpawnParams.ObjectFlags &= ~RF_Transactional;
		
		SimpleMeshPool[Chunk] = GetWorld()->SpawnActor<AFGVoxelSimpleChunkMesh>(SpawnParams);
		SimpleMeshFreelist[Chunk] = Chunk;
	}

	/**
	 * Our opportunity to signal that a chunk has finished loading, and if we have
	 * a mapping for it then we should mesh it!
	 */
	VoxSys->OnRenderCoordinatesFinishedLoading.AddWeakLambda(this, [this](TArray<FIntVector> LoadedCoordinates)
	{
		for(FIntVector& Coordinate : LoadedCoordinates)
		{
			if(SimpleMeshMappings.Contains(Coordinate))
			{
				SimpleMeshMappings.FindChecked(Coordinate)->GenerateMesh();
			}
		}
	});

	/**
	 * When a render coordinate is added (NOTE this only means we signalled to load the chunks)
	 * Create the chunks a mapping and pull it an actor from the pool.
	 */
	VoxSys->OnRenderCoordinatesAdded.AddWeakLambda(this, [this](TArray<FIntVector> AddedCoordinates)
	{
		auto& VoxelGrid = GetWorld()->GetSubsystem<UFGVoxelSystem>()->VoxelGrid;
		
		for(FIntVector& Coordinate : AddedCoordinates)
		{
			checkf(!SimpleMeshMappings.Contains(Coordinate), TEXT("Added coordinate already exists in mesh pool!"));
			const int32 NextFree = SimpleMeshFreelist.Pop();
			SimpleMeshMappings.Add(Coordinate, SimpleMeshPool[NextFree]);
			SimpleMeshPool[NextFree]->SetActorLocation(UFGVoxelUtils::ChunkCoordToVector(Coordinate));
			SimpleMeshPool[NextFree]->ChunkHandle = VoxelGrid->FindChunkChecked(Coordinate);
		}
	});

	/**
	 * When a render coordinate is removed from the render volume, we no longer need it!
	 * Reset the actor to factory settings and return it to the pool.
	 */
	VoxSys->OnRenderCoordinatesRemoved.AddWeakLambda(this, [this](TArray<FIntVector> RemovedCoordinates)
	{
		TArray<int32> FreedIndices;
		FreedIndices.Reserve(RemovedCoordinates.Num());
		
		for(FIntVector& Coordinate : RemovedCoordinates)
        {
        	int32 Freed = SimpleMeshPool.Find(SimpleMeshMappings.FindChecked(Coordinate));
			checkf(Freed != INDEX_NONE, TEXT("Removed coordinate not found in mesh pool!"));

			SimpleMeshPool[Freed]->ClearMesh();
			SimpleMeshPool[Freed]->ChunkHandle.Reset();
			SimpleMeshMappings.Remove(Coordinate);
			FreedIndices.Emplace(Freed);
        }
		SimpleMeshFreelist.Append(FreedIndices);
	});
}

void AFGVoxelSimpleMesher::Deinitialize()
{
	Super::Deinitialize();

	for(AFGVoxelSimpleChunkMesh* SimpleMesh : SimpleMeshPool)
	{
		SimpleMesh->Destroy();
	}
}

void AFGVoxelSimpleMesher::GenerateMesh(FIntVector ChunkCoordinate)
{
	SimpleMeshMappings.FindChecked(ChunkCoordinate)->GenerateMesh();
}

void AFGVoxelSimpleMesher::ClearMesh(FIntVector ChunkCoordinate)
{
	SimpleMeshMappings.FindChecked(ChunkCoordinate)->ClearMesh();
}
