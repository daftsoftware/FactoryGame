// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelGrid.h"
#include "FGVoxelUtils.h"

namespace FG
{
	static int32 ChunkGenBudget = 512;
	FAutoConsoleVariableRef CVarChunkGenBudget (
		TEXT("FG.ChunkGenBudget"),
		ChunkGenBudget,
		TEXT("How many chunks can be generated per tick."),
		ECVF_Default
	);

	static bool DebugChunkLoading = false;
	FAutoConsoleVariableRef CVarDebugChunkLoading (
		TEXT("FG.DebugChunkLoading"),
		DebugChunkLoading,
		TEXT("Draws debug boxes for chunk loading. (0/1)"),
		ECVF_Default
	);

	static bool VoxelImmediateMode = false;
	FAutoConsoleVariableRef CVarVoxelImmediateMode(
		TEXT("FG.VoxelImmediateMode"),
		VoxelImmediateMode,
		TEXT("Disables async loading or any non immediate generation."),
		ECVF_Default
	);
}

using namespace FG::Const;

/**
 * @TODO:
 * A slight problem with this design i've just noticed is that if you mark something to be generated
 * as a batch and it was already generated, it's skipped - you still get the callback in this case.
 * But what happens if you mark a chunk to be generated and it's already pending, you are now blocked
 * but some other load that happens, we might need a way to essentially check if it's pending and then
 * say OI THIS IS MINE NOW, and yoink it out of the other batch (the latermost loaded one).
 *
 * I think the big brain solution here is task stealing but that's too large brained 5 me.
 *
 * @TODO: UPDATE
 * Okay - now we've refactored a bit, a more sane solution has presented itself.
 * 
 * The simple way is just to track the load batch on the chunk handle itself and if another chunk with
 * a higher priority swoops in then we shuffle the priority up.
 *
 * Otherwise we could just have a single
 * 
 */
void UFGVoxelGrid::Tick(float DeltaTime)
{
	if (!GetWorld() || GetWorld()->bIsTearingDown)
	{
		return;
	}

	// Process garbage collection.
	while(!InternalGarbageChunks.IsEmpty())
	{
		OnUnloadedChunk.Broadcast(InternalGarbageChunks.Pop());
	}

	// Resize internal chunk allocations.
	int32 ExpansionSize = InternalChunkCount - InternalChunkData.Num();
	if(ExpansionSize > 0)
	{
		InternalChunkData.AddDefaulted();
	}

	if (FG::VoxelImmediateMode) // Immediate mode - single-threaded, non time-sliced loading.
	{
		int32 Elem = 0;

		for (FFGVoxelLoadHandle& LoadHandle : InternalLoadHandles)
		{
			for(int32 Load = LoadHandle->GetLoadCount(); Load < LoadHandle->GetBatchSize(); Load++, Elem++)
			{
				GenerateChunk(LoadHandle->ChunkHandles[Load]);
				LoadHandle->OnFinishedLoadingChunk.Broadcast(LoadHandle->ChunkHandles[Load]);
			}
			LoadHandle->OnFinishedLoadingBatch.Broadcast(MoveTemp(LoadHandle->ChunkHandles));
		}
		
		InternalLoadHandles.Empty();
		InternalNumPending = 0;
	}
	else // Parallel mode - multi-threaded, time-sliced loading.
	{
		TArray<FFGChunkHandle> WorkForFrame;
		TArray<int32> WorkBatches; // @TODO: Not ideal but quick and dirty.
		int32 BatchesToProcess = 0;

		// Squash batches into one workload capped at budget size.
		{
			int32 GenerationBudget = FMath::Min(InternalNumPending, FG::ChunkGenBudget);
			int32 RemainingBudget = GenerationBudget;
			int32 ProcessedElemsTotal = 0;

			for(int32 Batch = 0; Batch < InternalLoadHandles.Num(); Batch++) // Loop batches until we exceed budget.
			{
				if(RemainingBudget <= 0 || ProcessedElemsTotal >= InternalNumPending) // Exceeded budget or ran out of work.
				{
					break;
				}

				FFGVoxelLoadHandle LoadBatch = InternalLoadHandles[Batch];
				int32 ProcessedElemsInBatch = 0;

				for(int32 Chunk = LoadBatch->GetLoadCount(); Chunk < LoadBatch->GetBatchSize(); Chunk++) // Loop batch elems.
				{
					const FIntVector ChunkCoordinate = LoadBatch->ChunkCoordinates[Chunk];
					
					ProcessedElemsTotal += 1;
					ProcessedElemsInBatch += 1;

					if(IsChunkGenerated(ChunkCoordinate)) // Already loaded, skip.
					{
						continue;
					}

					if(FG::DebugChunkLoading)
					{
						UFGVoxelUtils::DebugDrawChunk(GetWorld(), ChunkCoordinate);
					}

					RemainingBudget -= 1;
					WorkForFrame.Add(LoadBatch->ChunkHandles[Chunk]);
					WorkBatches.Add(Batch);
					LoadBatch->LoadedIndices.Add(Chunk);

					if(RemainingBudget <= 0 || ProcessedElemsTotal >= InternalNumPending) // Exceeded budget or ran out of work.
					{
						break;
					}
				}

				LoadBatch->LoadCount += ProcessedElemsInBatch;
				BatchesToProcess += 1;
			}

			if(ProcessedElemsTotal > 0)
			{
				InternalNumPending -= ProcessedElemsTotal;
				InternalChunkData.AddDefaulted(WorkForFrame.Num());
				ActiveChunkHandles.Reserve(ActiveChunkHandles.Num() + WorkForFrame.Num());
			}
		}

		// Fork the work across threads.
		{
			// @TODO: We actually don't need to hold the game thread here, we could be trigger a UE::Task to
			// the gamethread here that fires callbacks and then the GT can continue bing chilling.

			DECLARE_CYCLE_STAT(TEXT("FGameThreadVoxelTask.ParallelGeneration"), STAT_VoxelParallelGeneration, STATGROUP_TaskGraphTasks);

			ParallelFor(WorkForFrame.Num(), [&](int32 Index) // BRRRRRRRRR
			{
				FFGVoxelLoadHandle LoadHandle = InternalLoadHandles[WorkBatches[Index]];
				FFGChunkHandle ChunkHandle = WorkForFrame[Index];
				
				GenerateChunk(ChunkHandle);

				UFGVoxelUtils::RunCommandOnGameThread(this, [this, LoadHandle, ChunkHandle]()
				{
					LoadHandle->OnFinishedLoadingChunk.Broadcast(ChunkHandle);
					
				}, GET_STATID(STAT_VoxelParallelGeneration));
			});

			int32 CompletedBatches = 0;

			// Run callbacks for batch work completion.
			for(int32 Batch = 0; Batch < BatchesToProcess; Batch++)
			{
				checkf(InternalLoadHandles.IsValidIndex(Batch), TEXT("Invalid batch index!"));
				FFGVoxelLoadHandle LoadHandle = InternalLoadHandles[Batch];
				
				if(InternalLoadHandles[Batch]->GetLoadCount() >= InternalLoadHandles[Batch]->GetBatchSize())
				{
					CompletedBatches += 1;
					LoadHandle->OnFinishedLoadingBatch.Broadcast(MoveTemp(LoadHandle->ChunkHandles));
				}
			}

			InternalLoadHandles.RemoveAt(0, CompletedBatches);
		}
	}
}


void UFGVoxelGrid::SetGeneratorType(TSubclassOf<UFGVoxelGenerator> GeneratorType)
{
	WorldGenerator = NewObject<UFGVoxelGenerator>(this, GeneratorType);
	FlushAllChunks();
}

FFGVoxelLoadHandle UFGVoxelGrid::LoadChunkAsync(FIntVector ChunkCoordinate)
{
	DECLARE_CYCLE_STAT(TEXT("FGameThreadVoxelTask.LoadChunkAsync"), STAT_VoxelLoadChunkAsync, STATGROUP_TaskGraphTasks);

	FFGVoxelLoadHandle LoadHandle = MakeShared<FFGVoxelLoadHandleData, ESPMode::ThreadSafe>(FFGVoxelLoadHandleData());
	
	UFGVoxelUtils::RunCommandOnGameThread(this, [this, ChunkCoordinate, LoadHandle]()
	{
		int32 NextIndex;

		if(!InternalChunkFreelist.IsEmpty()) // Steal from the freelist.
		{
			NextIndex = InternalChunkFreelist.Pop();
		}
		else // Reserve our index to append to the chunk data.
		{
			NextIndex = FPlatformAtomics::InterlockedIncrement(&InternalChunkCount) - 1;
		}

		FPlatformAtomics::InterlockedIncrement(&InternalNumPending);

		FFGChunkHandle ChunkHandle = ConstructChunkHandle(ChunkCoordinate);
		ChunkHandle->ChunkCoordinate = ChunkCoordinate;
		ChunkHandle->ChunkDataIndex = NextIndex;

		TSharedRef<FFGVoxelLoadHandleData> HandleDataRef = LoadHandle.ToSharedRef();
		//FPlatformAtomics::InterlockedExchange(&HandleDataRef->BatchStartIndex, InternalNumPending);
		FPlatformAtomics::InterlockedExchange(&HandleDataRef->BatchSize, 1);
		
		// @TODO: These arrays aren't safe to read on other threads while we are writing, need a mutex.
		HandleDataRef->ChunkCoordinates.Add(ChunkCoordinate);
		HandleDataRef->ChunkHandles.Add(ChunkHandle);

		InternalLoadHandles.Add(LoadHandle);
		
	}, GET_STATID(STAT_VoxelLoadChunkAsync));
	
	return LoadHandle;
}

FFGVoxelLoadHandle UFGVoxelGrid::LoadChunkBatchAsync(TArray<FIntVector> ChunkCoordinates)
{
	DECLARE_CYCLE_STAT(TEXT("FGameThreadVoxelTask.LoadChunkBatchAsync"), STAT_VoxelLoadChunkBatchAsync, STATGROUP_TaskGraphTasks);

	FFGVoxelLoadHandle LoadHandle = MakeShared<FFGVoxelLoadHandleData, ESPMode::ThreadSafe>(FFGVoxelLoadHandleData());
	
	UFGVoxelUtils::RunCommandOnGameThread(this, [this, ChunkCoordinates, LoadHandle]() {
		ActiveChunkHandles.Reserve(ActiveChunkHandles.Num() + ChunkCoordinates.Num());

		TArray<FFGChunkHandle> ChunkHandles;
		ChunkHandles.Reserve(ChunkCoordinates.Num());

		for(int32 Chunk = 0; Chunk < ChunkCoordinates.Num(); Chunk++)
		{
			int32 NextIndex;

			if(!InternalChunkFreelist.IsEmpty()) // Steal from the freelist.
			{
				NextIndex = InternalChunkFreelist.Pop();
			}
			else // Reserve our index to append to the chunk data.
			{
				NextIndex = FPlatformAtomics::InterlockedIncrement(&InternalChunkCount) - 1;
			}

			FFGChunkHandle ChunkHandle = ConstructChunkHandle(ChunkCoordinates[Chunk]);
			ChunkHandle->ChunkCoordinate = ChunkCoordinates[Chunk];
			ChunkHandle->ChunkDataIndex = NextIndex;
			ChunkHandles.Add(ChunkHandle);
		}
		
		FPlatformAtomics::InterlockedAdd(&InternalNumPending, ChunkCoordinates.Num());

		TSharedRef<FFGVoxelLoadHandleData> HandleDataRef = LoadHandle.ToSharedRef();
		//FPlatformAtomics::InterlockedExchange(&HandleDataRef->BatchStartIndex, InternalNumPending);
		FPlatformAtomics::InterlockedExchange(&HandleDataRef->BatchSize, ChunkCoordinates.Num());

		// @TODO: These arrays aren't safe to read on other threads while we are writing, need a mutex.
		HandleDataRef->ChunkCoordinates = ChunkCoordinates;
		HandleDataRef->ChunkHandles = ChunkHandles;

		InternalLoadHandles.Add(LoadHandle);
		
	}, GET_STATID(STAT_VoxelLoadChunkBatchAsync));
	
	return LoadHandle;
}

void UFGVoxelGrid::FlushAllChunks()
{
	FPlatformAtomics::InterlockedExchange(&InternalChunkCount, 0);
	ActiveChunkHandles.Empty();
	InternalChunkData.Empty();
	InternalGarbageChunks.Empty();
	InternalChunkFreelist.Empty();
	InternalLoadHandles.Empty();
}

FFGChunkHandle UFGVoxelGrid::FindChunk(FIntVector ChunkCoordinate)
{
	checkf(IsInGameThread(), TEXT("Attempted to find chunks on non-game thread!"));
	if(TWeakPtr<FFGChunkHandleData>* WeakChunkHandle = ActiveChunkHandles.Find(ChunkCoordinate))
	{
		if(WeakChunkHandle->IsValid())
		{
			return WeakChunkHandle->Pin();
		}
	}
	return FFGChunkHandle();
}

FFGChunkHandle UFGVoxelGrid::FindChunkChecked(FIntVector ChunkCoordinate)
{
	checkf(IsInGameThread(), TEXT("Attempted to find chunks on non-game thread!"));

	TWeakPtr<FFGChunkHandleData> WeakChunkHandle = ActiveChunkHandles.FindChecked(ChunkCoordinate);
	checkf(WeakChunkHandle.IsValid(), TEXT("Invalid chunk handle!"));
	return WeakChunkHandle.Pin();
}

bool UFGVoxelGrid::IsChunkGenerated(FIntVector ChunkCoordinate)
{
	checkf(IsInGameThread(), TEXT("Attempted to find chunks on non-game thread!"));

	FFGChunkHandle ChunkHandle = FindChunk(ChunkCoordinate);
	if(ChunkHandle.IsValid())
	{
		return ChunkHandle->Generated;
		//return GetChunkDataUnsafe(ChunkHandle)->HasAnyFlags(EFGChunkFlags::Generated);
	}
	return false;
}

// @TODO: Maybe use a future / promise here instead of a mutex.
FFGVoxelChunk* UFGVoxelGrid::GetChunkDataSafe(int32 ChunkDataIndex)
{
	checkf(InternalChunkData.IsValidIndex(ChunkDataIndex), TEXT("Invalid chunk data index!"));
	FFGVoxelChunk* ChunkDataPtr = InternalChunkData.GetData() + ChunkDataIndex;
	//UE::TScopeLock Lock(ChunkDataPtr->ChunkLocked);
	return ChunkDataPtr;
}

// @TODO: Maybe use a future / promise here instead of a mutex.
FFGVoxelChunk* UFGVoxelGrid::GetChunkDataSafe(FFGChunkHandle ChunkHandle)
{
	checkf(InternalChunkData.IsValidIndex(ChunkHandle->ChunkDataIndex), TEXT("Invalid chunk data index!"));
	FFGVoxelChunk* ChunkDataPtr = InternalChunkData.GetData() + ChunkHandle->ChunkDataIndex;
    //UE::TScopeLock Lock(ChunkDataPtr->ChunkLocked);
    return ChunkDataPtr;
}

FFGVoxelChunk* UFGVoxelGrid::GetChunkDataUnsafe(int32 ChunkDataIndex)
{
	checkf(InternalChunkData.IsValidIndex(ChunkDataIndex), TEXT("Invalid chunk data index!"));
	return InternalChunkData.GetData() + ChunkDataIndex;
}

FFGVoxelChunk* UFGVoxelGrid::GetChunkDataUnsafe(FFGChunkHandle ChunkHandle)
{
	checkf(InternalChunkData.IsValidIndex(ChunkHandle->ChunkDataIndex), TEXT("Invalid chunk data index!"));
	return InternalChunkData.GetData() + ChunkHandle->ChunkDataIndex;
}

void UFGVoxelGrid::GenerateChunk(FFGChunkHandle ChunkHandle)
{
	checkf(WorldGenerator.IsSet(), TEXT("Generation called without valid generator!"));
	GetGenerator()->Generate(InternalChunkData, ChunkHandle);
	//GetChunkDataUnsafe(ChunkHandle)->SetFlags(EFGChunkFlags::Generated);
	ChunkHandle->Generated = true;
}

FFGChunkHandle UFGVoxelGrid::ConstructChunkHandle(FIntVector ChunkCoordinate)
{
	TWeakObjectPtr<UFGVoxelGrid> GridWeak = MakeWeakObjectPtr(this);
	auto ChunkHandleDeleter = [GridWeak](void* ObjectToDelete)
	{
		FFGChunkHandleData* ChunkData = reinterpret_cast<FFGChunkHandleData*>(ObjectToDelete);
		
		if(GridWeak.IsValid())
		{
			GridWeak->ActiveChunkHandles.Remove(ChunkData->ChunkCoordinate);
            GridWeak->InternalGarbageChunks.Push(ChunkData->ChunkCoordinate);
            GridWeak->InternalChunkFreelist.Push(ChunkData->ChunkDataIndex);
		}

		FMemory::Free(ObjectToDelete);
	};

	FFGChunkHandle OutChunkHandle = MakeShareable(new FFGChunkHandleData(), MoveTemp(ChunkHandleDeleter));
	ActiveChunkHandles.Add(ChunkCoordinate, OutChunkHandle);
	return OutChunkHandle;
}
