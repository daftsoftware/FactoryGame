// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Containers/FGVoxelGrid.h"
#include "GameplayTagContainer.h"
#include "FGVoxelSystem.generated.h"

class AFGVoxelActorManager;
class AFGVoxelMesher;

/**
 * Voxel System is a manager for more game code side voxel code like the
 * local player's render distance and deciding which chunks should be loaded.
 *
 * For the more low level code for tracking and managing chunk generation and
 * loading you should be looking at UFGVoxelGrid.
 */
UCLASS()
class FGVOXEL_API UFGVoxelSystem final : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:

	UFGVoxelSystem();

	//~ Begin Super
	void PostInitialize() override;
	void Deinitialize() override;
	void Tick(float DeltaTime) override;
	bool IsTickableInEditor() const override { return true; }
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFGVoxelSystem, STATGROUP_Tickables) }
	bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End Super

	void EnumerateVoxels();

	void InitializeRendering();
	void UpdateRenderDistance(uint32 RenderSizeX);
	void DrawDebugChunkData(const FIntVector& ChunkCoordinate);

	void ModifyVoxel(FIntVector ChunkCoordinate, FIntVector VoxelCoordinate, int32 NewValue);
	void BatchModifyVoxels(TArray<TPair<FIntVector, FIntVector>> VoxelPositions, int32 NewValue);
	void MarkForRemesh(const FIntVector& ChunkCoordinate);

	/**
	 * Mark the entire render volume as dirty, flushing the chunks.
	 * This is useful in cases where the entire render volume needs to
	 * be refreshed, e.g on a teleport or when render distance changes.
	 */
	void FlushRendering();
	
	TMulticastDelegate<void(TArray<FIntVector>)> OnRenderCoordinatesAdded;
	TMulticastDelegate<void(TArray<FIntVector>)> OnRenderCoordinatesRemoved;
	TMulticastDelegate<void(TArray<FIntVector>)> OnRenderCoordinatesFinishedLoading;
	TMulticastDelegate<void(FIntVector, FIntVector, int32, int32)> OnVoxelEdited;

	UPROPERTY(Transient)
	TObjectPtr<UFGVoxelGrid> VoxelGrid;

	UPROPERTY(Transient)
	TOptional<TObjectPtr<AFGVoxelMesher>> ActiveMesher;

	UPROPERTY(Transient)
	TObjectPtr<AFGVoxelActorManager> VoxelActorManager;

	TArray<FIntVector> PendingRemeshes;

private:

	TMap<FIntVector, FFGChunkHandle> RenderableHandles;

	bool					AwaitingForcedGeneration;
	bool					RenderingInvalidated;
	TOptional<FIntVector>	LastPlayerCoord;
};
