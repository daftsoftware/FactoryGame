// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Generators/FGVoxelGenerator.h"
#include "Containers/FGVoxelChunk.h"
#include "FGVoxelGrid.generated.h"

struct FFGChunkHandleData
{
	FIntVector ChunkCoordinate = FIntVector::ZeroValue;
	int32 ChunkDataIndex = INDEX_NONE;
	bool Generated = false;

	FFGChunkHandleData() = default;
};

using FFGChunkHandle = TSharedPtr<FFGChunkHandleData>;

/**
 * Handle for an asynchronous chunk load operation.
 * This type should be used as FFGVoxelLoadHandle rather than directly so that
 * copies of the actual handle may be made a passed around.
 */
class FGVOXEL_API FFGVoxelLoadHandleData
{
public:
	friend class UFGVoxelGrid;

	FFGVoxelLoadHandleData(int32 InBatchLength, FIntVector InChunkCoordinate, FFGChunkHandle InChunkHandle) :
		ChunkHandles({ InChunkHandle }),
		ChunkCoordinates({ InChunkCoordinate }),
		BatchSize(InBatchLength),
		LoadCount(0),
		LoadedIndices({})
	{}

	FFGVoxelLoadHandleData(int32 InBatchLength, TArray<FIntVector> InChunkCoordinates, TArray<FFGChunkHandle> InChunkHandles) :
		ChunkHandles(InChunkHandles),
		ChunkCoordinates(InChunkCoordinates),
		BatchSize(InBatchLength),
		LoadCount(0),
		LoadedIndices({})
	{}

	TMulticastDelegate<void(FFGChunkHandle)>			OnFinishedLoadingChunk;
	TMulticastDelegate<void(TArray<FFGChunkHandle>)>	OnFinishedLoadingBatch;

	int32 GetLoadCount() const
	{
		return LoadCount;
	}

	int32 GetBatchSize() const
	{
		return BatchSize;
	}

	TArray<FFGChunkHandle>	ChunkHandles;
	TArray<FIntVector>		ChunkCoordinates;

private:

	FFGVoxelLoadHandleData() = default;

	volatile int32          BatchSize;
	volatile int32			LoadCount;
	TArray<int32>			LoadedIndices;
};

using FFGVoxelLoadHandle = TSharedPtr<FFGVoxelLoadHandleData>;

/**
 * Data grid for sparse voxel data on the CPU.
 *
 * It uses a virtual arena allocator internally to store chunk data.
 * This greatly simplifies memory management and heavily increases
 * cache performance, especially when there is many chunks of varying
 * compression sizes loaded.
 *
 * Our chunk sizes scale as power of two and the arena allocator holds
 * a pool for each of these sizes and will attempt to pool them together
 * into linear ranges on virtual address space, meaning that if you were
 * to iterate over all chunks in the entire grid linearly, you will only
 * theoretically cache miss once when you switch to a new chunk size.
 *
 * Ideally we will map out the virtual address ranges for our pools and
 * hold a ptr to the end of the allocation of the pools, letting us
 * gradually prefetch the entire range of chunks in each pool.
 */
UCLASS()
class FGVOXEL_API UFGVoxelGrid : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:
	
	//~ Begin Super
	UWorld* GetWorld() const override { return GetOuter()->GetWorld(); }
	void Tick(float DeltaTime) override;
	bool IsTickableInEditor() const override { return true; }
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFGVoxelGrid, STATGROUP_Tickables) }
	//~ End Super

	/**
	 * Sets the world generation class - aka regular or superflat.
	 * @param GeneratorType - The generator class to use.
	 */
	void SetGeneratorType(TSubclassOf<UFGVoxelGenerator> GeneratorType);

	/**
	 * Is the generator set?
	 * @return true if the generator is set.
	 */
	bool HasGenerator() const { return WorldGenerator.IsSet(); }

	/**
	 * Get the world generator instance.
	 * @returns The current world generator.
	 */
	UFGVoxelGenerator* GetGenerator() const { return WorldGenerator.GetValue(); }

	/**
	 * Mark a chunk to be loaded or generated.
	 * @param ChunkCoordinate - The chunk coordinate to load.
	 * @return LoadHandle - Handle to the load request.
	 */
	FFGVoxelLoadHandle LoadChunkAsync(FIntVector ChunkCoordinate);

	/**
	 * Mark a batch of chunks to be loaded or generated.
	 * @param ChunkCoordinates - Array of chunk coordinates to load.
	 * @return LoadHandle - Handle to the load request.
	 */
	FFGVoxelLoadHandle LoadChunkBatchAsync(TArray<FIntVector> ChunkCoordinates);

	/**
	 * Unload all chunks from memory forcefully!
	 */
	void FlushAllChunks();

	/**
	 * Find a chunks data by it's coordinate.
	 * @param ChunkCoordinate - The chunk coordinate to find.
	 * @return A potentially valid chunk handle.
	 */
	FFGChunkHandle FindChunk(FIntVector ChunkCoordinate);

	/**
	 * Find a chunks data by it's coordinate.
	 * @param ChunkCoordinate - The chunk coordinate to find.
	 * @return The found chunk handle.
	 */
	FFGChunkHandle FindChunkChecked(FIntVector ChunkCoordinate);

	/**
	 * Check if the chunk at the given coordinate has been generated or loaded.
	 * @param ChunkCoordinate - The chunk coordinate to check.
	 * @returns true if the chunk has been generated or loaded.
	 */
	bool IsChunkGenerated(FIntVector ChunkCoordinate);

	/**
	 * Safely get the chunk data at the given index, if the chunk is locked, the callee
	 * thread will be stalled out until the chunk unlocks. Avoid using this where possible
	 * since this can cause hitching - don't just use the unsafe version, be sensible with
	 * your access patterns.
	 * 
	 * This reference is not safe to store.
	 * 
	 * @param ChunkDataIndex - The index of the chunk data to get.
	 * @returns The chunk data at the given index.
	 */
	FFGVoxelChunk* GetChunkDataSafe(int32 ChunkDataIndex);

	/**
	 * Get the chunk data at the given index, this is unsafe and should only be used when
	 * you can absolutely guarantee that the chunk data is not being written on another thread.
	 * 
	 * This reference is not safe to store.
	 * 
	 * @param ChunkHandle - The handle of the chunk data to get.
	 * @return The chunk data from the given handle.
	 */
	FFGVoxelChunk* GetChunkDataSafe(FFGChunkHandle ChunkHandle);

	/**
	 * Get the chunk data at the given index, this is unsafe and should only be used when
	 * you can absolutely guarantee that the chunk data is not being written on another thread.
	 * 
	 * This reference is not safe to store.
	 * 
	 * @param ChunkDataIndex - The index of the chunk data to get.
	 * @return The chunk data at the given index.
	 */
	FFGVoxelChunk* GetChunkDataUnsafe(int32 ChunkDataIndex);

	/**
	 * Get the chunk data at the given index, this is unsafe and should only be used when
	 * you can absolutely guarantee that the chunk data is not being written on another thread.
	 * 
	 * This reference is not safe to store.
	 * 
	 * @param ChunkHandle - The handle of the chunk data to get.
	 * @return The chunk data from the given handle.
	 */
	FFGVoxelChunk* GetChunkDataUnsafe(FFGChunkHandle ChunkHandle);

private:
	
	/**
	 * Generate a chunk's data, loading saved data from memory if required.
	 * Used for multithreaded generation, therefore must be used with care
	 * only in the correct contexts. To externally use this function you
	 * should be using either LoadChunkAsync or LoadChunkSynchronous.
	 * @param ChunkHandle - The data that should be written to.
	 */
	void GenerateChunk(FFGChunkHandle ChunkHandle);

	TMulticastDelegate<void(FIntVector)> OnUnloadedChunk;
	
	UPROPERTY(Transient)
	TOptional<TObjectPtr<UFGVoxelGenerator>> WorldGenerator;
	
	FFGChunkHandle ConstructChunkHandle(FIntVector ChunkCoordinate);
	
	TMap<FIntVector, TWeakPtr<FFGChunkHandleData>> ActiveChunkHandles;

	// @TODO: Review default ctor! Unitinialized values
	// @TODO: Don't think we wanna use mutex for chunk locking, use futures.
	
	int32							InternalNumPending;
	volatile int32					InternalChunkCount;
	TArray<FFGVoxelChunk>	        InternalChunkData;
	TArray<FIntVector>				InternalGarbageChunks;
	TArray<int32>			        InternalChunkFreelist;
	TArray<FFGVoxelLoadHandle>		InternalLoadHandles;
};
