// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGVoxelDefines.h"
#include "FGVoxelUtils.h"
#include "Containers/FGVoxelGrid.h"
#include "GameStateSubsystem.h"
#include "Components/ControllerComponent.h"
#include "FGVoxelNetManager.generated.h"

class UFGVoxelNetManager;

/**
 * @TODO: Mark brings up a valid point which is that I maybe got it right the
 * first time around.
 *
 * First of all - the idea of using a version number to determine chunks to send
 * is a YAGNI - It's a memory and storage tradeoff, and if we learned anything so
 * far it's that memory is the bottleneck everytime. We can actually just do a simple
 * hash of the chunk data by adding together block id's. Mark recommends to do this
 * parallel but I think that we want our server to be able to go single threaded.
 *
 * The problem with this is that we can't hash until we have a chunk loaded so we
 * are totally bottlenecked on IO here. Also if we don't hash and instead go version
 * numbers route, we are essentially just trading off for a smaller IO operation.
 *
 * Mark seems to think that minecraft is just sending the chunk data literally
 * whenever you enter a new chunk, and doing this entirely serverside. This is
 * actually pretty reasonable if you're thinking about things from a server perf
 * perspective rather than a bandwidth one.
 *
 * However, something to consider is that we haven't profiled it, and we are yet
 * again in a situation that requires R&D to figure out. I am inclined to have the
 * opinion that we should just do it the minecraft way, and partially remove the
 * code I wrote here - reason being I want something working fast and that method
 * has seemingly worked for minecraft for like 15 years so how bad can it be?
 *
 * That being said, before this decision is made we should absolutely check to the
 * best of our ability that Mark is correct here and that minecraft definitely does
 * not do any kind of client side hashing or anything like that.
 */

/**
 * How voxel networking works:
 *
 * Client sends server a chunk coordinate (pawn location quantized), and the
 * version numbers in the "relevancy bubble" (server render distance) around it.
 *
 * Server calculates which chunks are "observed" by any client by doing the
 * intersection of all clients relevancy bubbles (including listen servers).
 *
 * Server loads / generates all observed chunks if they aren't already loaded.
 *
 * Server processes factories (run factory graph ticks).
 * Server processes entities (run entity system ticks).
 * Server does any chunk updates (voxel edits) that are pending.
 *
 * Server calculates which chunks are dirty based on local version numbers against
 * client version numbers, then sends client outdated chunk data via a singular RPC.
 * The reason we use a singular RPC here is because with Iris there isn't a need for
 * fast array serializer or other more complex methods because now we have partial
 * sequential net blobs which automatically break up big RPCs and stream them.
 * Payload data structure is the following:
 * - Interval tree for which chunk indexes got updated inside the bubble
 * - Raw chunk palette data
 *
 * Client receives chunk update payload, places updates into local cache.
 *
 * Client ticks, applying any updates from the local cache to loaded chunks.
 */

/**
 * Reversible Net Reference for a singular Voxel.
 * 
 * This is used to uniquely identify voxels in the world
 * over the network. Aka if a player removes or adds a voxel
 * in a chunk, we need to know which voxel we are talking about.
 *
 * We also use this to map entity references over the network.
 * Aka item containers like chests use this as their stable name
 * on the network so that replication is automatically mapped to
 * specific voxels.
 */
USTRUCT()
struct FFGVoxelNetRef final
{
	GENERATED_BODY()

	FIntVector ChunkPosWS;
	uint16 VoxelIndex;

	FFGVoxelNetRef()
		: ChunkPosWS(FIntVector::ZeroValue),
		VoxelIndex(0)
	{}

	FFGVoxelNetRef(FIntVector InChunkPosWS, uint16 InVoxelIndex)
		: ChunkPosWS(InChunkPosWS),
		VoxelIndex(InVoxelIndex)
	{}

	FFGVoxelNetRef(FIntVector ChunkCoordinate, FIntVector VoxelCoordinate)
		: ChunkPosWS(ChunkCoordinate),
		VoxelIndex(UFGVoxelUtils::FlattenVoxelCoord(VoxelCoordinate))
	{}

	UScriptStruct* GetScriptStruct() const { return StaticStruct(); }

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		Ar << ChunkPosWS;
		Ar << VoxelIndex;
		return true;
	}
};

struct FFGVoxelEdit
{
	APlayerController* Instigator;
	FIntVector ChunkCoordinate;
	FIntVector VoxelCoordinate;
	int32 NewVoxelType;
};

USTRUCT()
struct FFGChunkUpdatePayload
{
	GENERATED_BODY()
	
	TArray<FIntVector>	ChunkPositions;
	TArray<FFGVoxelChunk>	ChunkData;
};

/**
 * GameState Extension for Voxel Networking.
 */
UCLASS(MinimalAPI)
class UFGVoxelGameStateSubsystem : public UTickableGameStateSubsystem
{
	GENERATED_BODY()
public:

	friend class UFGVoxelNetManager;

	//~ Begin Super
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFGVoxelGameState, STATGROUP_Tickables); }
	//~ End Super

	void CalculateChangelists(TSet<FIntVector>& IntersectedAdds, TSet<FIntVector>& IntersectedRemoves);

	TMap<TWeakObjectPtr<APlayerController>, TOptional<FIntVector>>	PlayerLastCoords;
	TMap<TWeakObjectPtr<APlayerController>, TSet<FIntVector>>		PendingClientUpdates;
	
	TMap<FIntVector, FFGChunkHandle>		TrackedChunks;
	TMap<FIntVector, TArray<FFGVoxelEdit>>	PendingVoxelEdits;

	TMulticastDelegate<void(FFGVoxelEdit)> VoxelEditValidated;
	TMulticastDelegate<void(FFGVoxelEdit)> VoxelEditFailed;
};

/**
 * PlayerController extension for Voxel Networking.
 */
UCLASS()
class FGVOXEL_API UFGVoxelNetManager : public UControllerComponent
{
	GENERATED_BODY()
public:

	friend class UFGVoxelGameStateSubsystem;
	
	UFGVoxelNetManager(const FObjectInitializer& ObjectInitializer);

	void MoveMeToGA_MakeEdit(FIntVector ChunkCoordinate, FIntVector VoxelCoordinate, int32 NewValue);

	/** Pending changelists to apply to chunks next time they are seen. */
	TMap<FIntVector, FFGVoxelChunk> PendingChunkUpdates;

	UFUNCTION(Client, Reliable)
	void ClientReceiveAddressedVoxelUpdate(const FFGVoxelNetRef& Address, uint32 NewVoxelType);
};
