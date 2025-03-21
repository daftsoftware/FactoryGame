// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/Actor.h"
#include "FGVoxelActorManager.generated.h"

using FFGVoxelRef = TPair<FIntVector, FIntVector>;

UCLASS(Abstract)
class FGVOXEL_API AFGVoxelActor : public AActor
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly)
	int32 VoxelType;

	UPROPERTY(BlueprintReadOnly)
	FIntVector ChunkCoordinate;

	UPROPERTY(BlueprintReadOnly)
	FIntVector VoxelCoordinate;
};

/**
 * Actor Manager is responsible for hooking into chunk loads / unloads
 * and creating required actors from voxel data. You can think of it kind
 * of like a parser, which figures out which actors map to which ID's and
 * then manages their lifetimes.
 *
 * Voxel related entities have stable network names based on their voxel net
 * ref.
 */
UCLASS()
class FGVOXEL_API AFGVoxelActorManager : public AActor
{
	GENERATED_BODY()
public:

	//~ Begin Super
	virtual void BeginPlay() override;
	//~ End Super
	
	virtual void EnumerateClassMappings();
	virtual void OnChunkLoaded(FIntVector ChunkCoordinate);
	virtual void OnChunkUnloaded(FIntVector ChunkCoordinate);
	virtual void OnVoxelModified(FIntVector ChunkCoordinate, FIntVector VoxelCoordinate, int32 OldValue, int32 NewValue);

	TMap<int32, TSoftClassPtr<AFGVoxelActor>> ClassMappings;
	TMap<FFGVoxelRef, TObjectPtr<AFGVoxelActor>> ActorMappings;
};