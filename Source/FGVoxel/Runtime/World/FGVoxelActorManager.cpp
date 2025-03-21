// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelActorManager.h"
#include "Misc/FGVoxelMetadata.h"
#include "World/FGVoxelSystem.h"
#include "FGVoxelUtils.h"
#include "Engine/AssetManager.h"

void AFGVoxelActorManager::BeginPlay()
{
	Super::BeginPlay();

	auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();
	VoxSys->OnVoxelEdited.AddUObject(this, &ThisClass::OnVoxelModified);

	VoxSys->OnRenderCoordinatesFinishedLoading.AddWeakLambda(this, [this](TArray<FIntVector> Coordinates)
	{
		for(FIntVector Coordinate : Coordinates)
		{
			OnChunkLoaded(Coordinate);
		}
	});
	
	VoxSys->OnRenderCoordinatesRemoved.AddWeakLambda(this, [this](TArray<FIntVector> Coordinates)
	{
		for(FIntVector Coordinate : Coordinates)
		{
			OnChunkUnloaded(Coordinate);
		}
	});
}

void AFGVoxelActorManager::EnumerateClassMappings()
{
	ClassMappings.Empty();
	
	TArray<FPrimaryAssetId> VoxelMetadataAssets;
	
	UAssetManager::Get().GetPrimaryAssetIdList(
		FPrimaryAssetType("VoxelMetadata"),
		VoxelMetadataAssets,
		EAssetManagerFilter::Default
	);

	for(FPrimaryAssetId AssetId : VoxelMetadataAssets)
	{
		TSoftObjectPtr<UFGVoxelMetadata> MetadataAsset = TSoftObjectPtr<UFGVoxelMetadata>(
			UAssetManager::Get().GetPrimaryAssetPath(AssetId));

		if(UFGVoxelMetadata* Metadata = MetadataAsset.LoadSynchronous())
		{
			if(Metadata->ActorClass.IsSet())
			{
				int32 VoxelInternalId = GVoxelTypeMap.FindChecked(Metadata->VoxelName);
				ClassMappings.Add(VoxelInternalId, Metadata->ActorClass.GetValue());
			}
		}
	}
}

void AFGVoxelActorManager::OnChunkLoaded(FIntVector ChunkCoordinate)
{
	auto& VoxelGrid = GetWorld()->GetSubsystem<UFGVoxelSystem>()->VoxelGrid;
	FFGChunkHandle ChunkHandle = VoxelGrid->FindChunkChecked(ChunkCoordinate);

	for(int32 Voxel = 0; Voxel < FG::Const::ChunkSizeXYZ; Voxel++)
	{
		FIntVector VoxelCoordinate = UFGVoxelUtils::UnflattenVoxelCoord(Voxel);
		int32 VoxelType = VoxelGrid->GetChunkDataUnsafe(ChunkHandle)->GetVoxel(Voxel);

		if(ClassMappings.Contains(VoxelType))
		{
			TSoftClassPtr<AFGVoxelActor> Class = ClassMappings.FindChecked(VoxelType);
			TObjectPtr<AFGVoxelActor>& ChunkActor = ActorMappings.FindOrAdd(FFGVoxelRef(ChunkCoordinate, VoxelCoordinate));
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.ObjectFlags |=	RF_Transient;
			SpawnParams.ObjectFlags &= ~RF_Transactional;

			auto* Actor = GetWorld()->SpawnActor<AFGVoxelActor>(Class.LoadSynchronous(), SpawnParams);
			ChunkActor = Actor;
		}
	}
}

void AFGVoxelActorManager::OnChunkUnloaded(FIntVector ChunkCoordinate)
{
	for(int32 Voxel = 0; Voxel < FG::Const::ChunkSizeXYZ; Voxel++)
	{
		FIntVector VoxelCoordinate = UFGVoxelUtils::UnflattenVoxelCoord(Voxel);

		if(ActorMappings.Contains(FFGVoxelRef(ChunkCoordinate, VoxelCoordinate)))
		{
			ActorMappings[FFGVoxelRef(ChunkCoordinate, VoxelCoordinate)]->Destroy();
			ActorMappings.Remove(FFGVoxelRef(ChunkCoordinate, VoxelCoordinate));
		}
	}
}

void AFGVoxelActorManager::OnVoxelModified(FIntVector ChunkCoordinate, FIntVector VoxelCoordinate, int32 OldValue, int32 NewValue)
{
	if(ClassMappings.Contains(OldValue)) // Delete old actor.
	{
		if(ActorMappings.Contains(FFGVoxelRef(ChunkCoordinate, VoxelCoordinate)))
		{
			ActorMappings[FFGVoxelRef(ChunkCoordinate, VoxelCoordinate)]->Destroy();
			ActorMappings.Remove(FFGVoxelRef(ChunkCoordinate, VoxelCoordinate));
		}
	}

	if(ClassMappings.Contains(NewValue)) // Create new actor.
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |=	RF_Transient;
		SpawnParams.ObjectFlags &= ~RF_Transactional;

		FVector Location = UFGVoxelUtils::ChunkCoordToVector(ChunkCoordinate)
			+ FVector(VoxelCoordinate * FG::Const::VoxelSizeUU)
			+ FVector(FG::Const::VoxelSizeUU / 2);
		
		FTransform ActorTransform(
			FRotator::ZeroRotator,
			Location,
			FVector::OneVector);

		for(auto Class : ClassMappings)
		{
			UE_LOG(LogTemp, Warning, TEXT("Class: %s"), *Class.Value.LoadSynchronous()->GetName());
		}

		UClass* ActorClass = ClassMappings.FindChecked(NewValue).LoadSynchronous();
		
		checkf(ActorClass, TEXT("Actor class not found for voxel type %d"), NewValue);
		auto* VoxelActor = GetWorld()->SpawnActor<AFGVoxelActor>(ActorClass, ActorTransform, SpawnParams);
		
		VoxelActor->ChunkCoordinate = ChunkCoordinate;
		VoxelActor->VoxelCoordinate = VoxelCoordinate;
		VoxelActor->VoxelType = NewValue;
		
		ActorMappings.Add(FFGVoxelRef(ChunkCoordinate, VoxelCoordinate), VoxelActor);
	}
}
