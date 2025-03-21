// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelCulledMesher.h"
#include "FGVoxelCulledMeshComponent.h"
#include "FGVoxelDefines.h"
#include "FGVoxelUtils.h"
#include "World/FGVoxelSystem.h"

AFGVoxelCulledMesher::AFGVoxelCulledMesher()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	bListedInSceneOutliner = false;
#endif

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
}

void AFGVoxelCulledMesher::Initialize()
{
	Super::Initialize();

	auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();

	MeshPool.AddUninitialized(GRenderSizeXYZ);
	Freelist.AddUninitialized(GRenderSizeXYZ);
	
	for(int32 Chunk = 0; Chunk < GRenderSizeXYZ; Chunk++)
	{
		EObjectFlags MeshFlags = RF_NoFlags;
		MeshFlags |= RF_Transient;
		MeshFlags &= ~RF_Transactional;

		auto* MeshComponent = NewObject<UFGVoxelCulledMeshComponent>(
			this, *FString::Format(TEXT("{CulledMesh{0}"), {Chunk}), MeshFlags);

		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetUsingAbsoluteLocation(true);
		MeshComponent->SetUsingAbsoluteRotation(true);
		MeshComponent->SetUsingAbsoluteScale(true);
		MeshComponent->RegisterComponent();

		MeshPool[Chunk] = MeshComponent;
		Freelist[Chunk] = Chunk;
	}

		/**
	 * Our opportunity to signal that a chunk has finished loading, and if we have
	 * a mapping for it then we should mesh it!
	 */
	VoxSys->OnRenderCoordinatesFinishedLoading.AddWeakLambda(this, [this](TArray<FIntVector> LoadedCoordinates)
	{
		for(FIntVector& Coordinate : LoadedCoordinates)
		{
			if(MeshMappings.Contains(Coordinate))
			{
				MeshMappings.FindChecked(Coordinate)->GenerateMesh();
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
			checkf(!MeshMappings.Contains(Coordinate), TEXT("Added coordinate already exists in mesh pool!"));
			const int32 NextFree = Freelist.Pop();
			MeshMappings.Add(Coordinate, MeshPool[NextFree]);
			MeshPool[NextFree]->SetWorldLocation(UFGVoxelUtils::ChunkCoordToVector(Coordinate));
			MeshPool[NextFree]->ChunkHandle = VoxelGrid->FindChunkChecked(Coordinate);
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
        	int32 Freed = MeshPool.Find(MeshMappings.FindChecked(Coordinate));
			checkf(Freed != INDEX_NONE, TEXT("Removed coordinate not found in mesh pool!"));

			MeshPool[Freed]->ClearMesh();
			MeshPool[Freed]->ChunkHandle.Reset();
			MeshMappings.Remove(Coordinate);
			FreedIndices.Emplace(Freed);
        }
		Freelist.Append(FreedIndices);
	});
}

void AFGVoxelCulledMesher::Deinitialize()
{
	Super::Deinitialize();
}

void AFGVoxelCulledMesher::GenerateMesh(FIntVector ChunkCoordinate)
{
	Super::GenerateMesh(ChunkCoordinate);

	MeshMappings.FindChecked(ChunkCoordinate)->GenerateMesh();
}

void AFGVoxelCulledMesher::ClearMesh(FIntVector ChunkCoordinate)
{
	Super::ClearMesh(ChunkCoordinate);

	MeshMappings.FindChecked(ChunkCoordinate)->ClearMesh();
}