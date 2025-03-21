// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelSystem.h"
#include "FGVoxelUtils.h"
#include "Meshers/FGVoxelMesher.h"
#include "Utils/FGUtils.h"
#include "Misc/FGVoxelProjectSettings.h"
#include "EngineUtils.h"
#include "FGDebugDraw.h"
#include "FGVoxelActorManager.h"
#include "Containers/FGVoxelChunk.h"
#include "FGVoxelCollisionManager.h"
#include "FGVoxelGameplayTags.h"
#include "Engine/AssetManager.h"
#include "Logging/StructuredLog.h"
#include "Misc/FGVoxelMetadata.h"

namespace FG
{
	static bool EnableVoxelMeshing = true;
	FAutoConsoleVariableRef CVarEnableVoxelMeshing (
		TEXT("FG.EnableVoxelMeshing"),
		EnableVoxelMeshing,
		TEXT("Enables rendering / meshing of voxels. (0/1)"),
		ECVF_Default
	);

	static bool DebugDrawVoxelChunkData = false;
	FAutoConsoleVariableRef CVarDebugDrawVoxelChunkData (
		TEXT("FG.DebugDrawVoxelChunkData"),
		DebugDrawVoxelChunkData,
		TEXT("Enables debug drawing of data of the chunk the player is stood on. (0/1)"),
		ECVF_Default
	);

	static bool DebugDrawVoxelRenderDiffs = false;
	FAutoConsoleVariableRef CVarDebugDrawVoxelRenderDiffs (
		TEXT("FG.DebugDrawVoxelRenderDiffs"),
		DebugDrawVoxelRenderDiffs,
		TEXT("Enables debug drawing of render adds / removes. (0/1)"),
		ECVF_Default
	);
	
	static bool FreezeChunkLoading = false;
	FAutoConsoleVariableRef CVarFreezeChunkLoading (
		TEXT("FG.FreezeChunkLoading"),
		FreezeChunkLoading,
		TEXT("Freezes chunk loading. (0/1)"),
		ECVF_Default
	);

	static int32 VoxelDefaultRenderDistance = 4;
	FAutoConsoleVariableRef CVarVoxelDefaultRenderDistance (
		TEXT("FG.VoxelRenderDistance"),
		VoxelDefaultRenderDistance,
		TEXT("Overrides the default render distance."),
		ECVF_Default
	);

	static FAutoConsoleCommandWithWorld CmdInvalidateRendering(
		TEXT("FG.FlushRendering"),
		TEXT("Flushes rendering chunks, reloading any chunks in the render volume."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			auto* VoxSys = World->GetSubsystem<UFGVoxelSystem>();
			VoxSys->FlushRendering();
		})
	);

	static FAutoConsoleCommandWithWorld CmdFlushLoadedChunks(
		TEXT("FG.FlushLoadedChunks"),
		TEXT("Forcefully flushes all loaded chunks from memory."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			auto* VoxSys = World->GetSubsystem<UFGVoxelSystem>();
			VoxSys->VoxelGrid->FlushAllChunks();
		})
	);

	static FAutoConsoleCommandWithWorld CmdDumpVoxelIds(
		TEXT("FG.DumpVoxelIds"),
		TEXT("Dump to log all voxel identifiers."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			for(auto Entry : GVoxelTypeMap)
			{
				UE_LOGFMT(LogTemp, Display, "{Tag} - {InternalId}", Entry.Key.ToString(), Entry.Value);
			}
		})
	);

	static FAutoConsoleCommandWithWorld CmdDumpChunkMemory(
		TEXT("FG.DumpChunkMemory"),
		TEXT("Dump chunk memory to log on the current player coordinate."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			const FTransform ViewXForm = UFGUtils::GetCameraViewTransform(World);
			const FIntVector PlayerCoord = UFGVoxelUtils::VectorToChunkCoord(ViewXForm.GetLocation());

			auto& VoxelGrid = World->GetSubsystem<UFGVoxelSystem>()->VoxelGrid;
			
			FFGChunkHandle ChunkHandle = VoxelGrid->FindChunk(PlayerCoord);

			if(ChunkHandle.IsValid())
			{
				FFGVoxelChunk* ChunkData = VoxelGrid->GetChunkDataSafe(ChunkHandle);

				FString DumpString;

				for(int32 Voxel = 0; Voxel < Const::ChunkSizeXYZ; Voxel++)
				{
					const FIntVector VoxelCoordinate = UFGVoxelUtils::UnflattenVoxelCoord(Voxel);
					DumpString.Append(FString::FromInt(ChunkData->GetVoxel(VoxelCoordinate)));
				}

				UE_LOGFMT(LogTemp, Warning, "Chunk Memory Dump: {DumpStr}", DumpString);
			}
		})
	);
}

UFGVoxelSystem::UFGVoxelSystem() :
	AwaitingForcedGeneration(true),
	RenderingInvalidated(false)
{
	VoxelGrid = CreateDefaultSubobject<UFGVoxelGrid>(TEXT("VoxelGrid"));
}

void UFGVoxelSystem::PostInitialize()
{
	Super::PostInitialize();

	// @TODO: Choose a generator from the player save, or fallback to default generator.

	const UFGVoxelProjectSettings* VoxelSettings = GetDefault<UFGVoxelProjectSettings>();

	if (UClass* DefaultGeneratorClass = VoxelSettings->VoxelDefaultGeneratorClass.LoadSynchronous())
	{
		// Set the world generator type used by the save.
		VoxelGrid->SetGeneratorType(DefaultGeneratorClass);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |=	RF_Transient;
	SpawnParams.ObjectFlags &= ~RF_Transactional;

	if(UClass* DefaultCollisionManagerClass = VoxelSettings->VoxelDefaultCollisionManagerClass.LoadSynchronous())
	{
		// Spawn a collision manager.
		GetWorld()->SpawnActor<AFGVoxelCollisionManager>(DefaultCollisionManagerClass, SpawnParams);
	}

	// Clear cached results from previous sessions.
	GVoxelTypeMap.Empty();

	// Spawn actor manager.
	VoxelActorManager = GetWorld()->SpawnActor<AFGVoxelActorManager>(AFGVoxelActorManager::StaticClass(), SpawnParams);
	
	InitializeRendering();
}

void UFGVoxelSystem::Deinitialize()
{
	Super::Deinitialize();

	for(auto* Renderer : TActorRange<AFGVoxelMesher>(GetWorld()))
	{
		Renderer->Deinitialize();
		Renderer->Destroy();
	}

	GVoxelTypeMap.Empty();
}

void UFGVoxelSystem::Tick(float DeltaTime)
{
	using namespace FG::Const;
	
	Super::Tick(DeltaTime);

	if(GVoxelTypeMap.IsEmpty()) // Enumerate voxel types when they are loaded.
	{
		EnumerateVoxels();
		return;
	}

	// Don't do anything if we are missing a mesher, generator or chunk loading is frozen.
	if(!ActiveMesher.IsSet() || !VoxelGrid->HasGenerator() || FG::FreezeChunkLoading)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, DeltaTime, FColor::Red,
			TEXT("Voxel System is missing a mesher, generator or chunk loading is frozen!"));

		return;
	}

	const FTransform ViewXForm = UFGUtils::GetCameraViewTransform(GetWorld());
	const FIntVector PlayerCoord = UFGVoxelUtils::VectorToChunkCoord(ViewXForm.GetLocation());

	if(!LastPlayerCoord.IsSet()) // We just spawned so no valid last coord to compare.
	{
		AwaitingForcedGeneration = true;	// Add all chunks in the render volume.
		LastPlayerCoord = PlayerCoord;		// There was no last coord, don't remove anything.
	}

	if(FG::DebugDrawVoxelChunkData) // Debug draw data for the current player chunk coord.
	{
		DrawDebugChunkData(PlayerCoord);
	}

	// Remesh any chunks that have been marked for remeshing.
	for(const FIntVector& ChunkCoordinate : PendingRemeshes)
	{
		ActiveMesher.GetValue()->ClearMesh(ChunkCoordinate);
		ActiveMesher.GetValue()->GenerateMesh(ChunkCoordinate);
	}
	PendingRemeshes.Empty();

	constexpr double ChunkSizeXUU = ChunkSizeX * VoxelSizeUU;
    constexpr double ChunkExtentXUU = ChunkSizeXUU / 2;
    
    FVector RenderVolumeExtent = FVector(GRenderSizeX * ChunkSizeX * VoxelSizeUU) / 2;
    FBox LastRenderVolume = FBox::BuildAABB(UFGVoxelUtils::ChunkCoordToVector(LastPlayerCoord.GetValue()), RenderVolumeExtent);
    FBox NewRenderVolume = FBox::BuildAABB(UFGVoxelUtils::ChunkCoordToVector(PlayerCoord), RenderVolumeExtent);

	if(PlayerCoord != LastPlayerCoord || AwaitingForcedGeneration) // Local player crossed chunk border or we forced reload.
	{
		if(FG::DebugDrawVoxelRenderDiffs)
		{
			FG::DebugDrawBox(
				GetWorld(),
				LastRenderVolume.GetCenter(),
				FQuat::Identity,
				GRenderSizeX * ChunkSizeX * VoxelSizeUU,
				FLinearColor::Blue,
				0.5);
			
			FG::DebugDrawBox(
				GetWorld(),
				NewRenderVolume.GetCenter(),
				FQuat::Identity,
				GRenderSizeX * ChunkSizeX * VoxelSizeUU,
				FLinearColor::White,
				0.5);
		}

		TArray<FIntVector> RenderAdditions;
		TArray<FIntVector> RenderRemovals;
		RenderAdditions.Reserve(GRenderSizeXYZ);
		RenderRemovals.Reserve(GRenderSizeXYZ);

		// Calculate removals from non-intersecting chunks in last render distance.
		for(double X = LastRenderVolume.Min.X; X < LastRenderVolume.Max.X; X += ChunkSizeXUU)
		{
			for(double Y = LastRenderVolume.Min.Y; Y < LastRenderVolume.Max.Y; Y += ChunkSizeXUU)
			{
				for(double Z = LastRenderVolume.Min.Z; Z < LastRenderVolume.Max.Z; Z += ChunkSizeXUU)
				{
					const FVector RemovalLocation = FVector(X, Y, Z) + ChunkExtentXUU;

					if(!FMath::PointBoxIntersection(RemovalLocation, NewRenderVolume))
					{
						RenderRemovals.Add(UFGVoxelUtils::VectorToChunkCoord(RemovalLocation));
					}
				}
			}
		}

		// Build a new load spiral (order to load chunks - load closest to player first)
		TArray<FIntVector2> LoadSpiral = UFGVoxelUtils::MakeMatrixSpiral2D(GRenderSizeX, GRenderSizeX);
		
		for(FIntVector2& SpiralCoordinate : LoadSpiral) // Iterate the XY plane.
		{
			// @TODO: This currently iterates upwards not downwards.
			for(double Z = NewRenderVolume.Min.Z; Z < NewRenderVolume.Max.Z; Z += ChunkSizeXUU) // Iterate chunk column downwards.
			{
				const FVector AdditionLocation = FVector(
					NewRenderVolume.Min.X + SpiralCoordinate.X * ChunkSizeXUU,
					NewRenderVolume.Min.Y + SpiralCoordinate.Y * ChunkSizeXUU,
					Z) + ChunkExtentXUU;

				if(!FMath::PointBoxIntersection(AdditionLocation, LastRenderVolume) || AwaitingForcedGeneration)
				{
					RenderAdditions.Add(UFGVoxelUtils::VectorToChunkCoord(AdditionLocation));
				}
			}
		}

		// Chunk fell outside render distance - remove renderable handles.
		for(FIntVector Removal : RenderRemovals)
		{
			RenderableHandles.Remove(Removal);
			
			if(FG::DebugDrawVoxelRenderDiffs)
			{
				UFGVoxelUtils::DebugDrawChunk(GetWorld(), Removal, FLinearColor::Red);
			}
		}
		OnRenderCoordinatesRemoved.Broadcast(MoveTemp(RenderRemovals));

		// @TODO: We need to reserve renderables, we can't just wait asyncronously to actually
		// add the coord because the player can move and destruct it before it's finished loading
		// in which case the renderer goes yo wtf are you talking about i've never met this man
		// in my life.

		// Async load anything that entered the render distance.
		FFGVoxelLoadHandle LoadHandle = VoxelGrid->LoadChunkBatchAsync(RenderAdditions);
		RenderableHandles.Reserve(RenderableHandles.Num() + RenderAdditions.Num());

		// Chunk in batch finished loading.
		LoadHandle->OnFinishedLoadingChunk.AddWeakLambda(this, [this](FFGChunkHandle LoadedChunk)
		{
			RenderableHandles.Add(LoadedChunk->ChunkCoordinate, LoadedChunk);
			OnRenderCoordinatesFinishedLoading.Broadcast({ LoadedChunk->ChunkCoordinate });
		});

		// Entire batch finished loading.
		LoadHandle->OnFinishedLoadingBatch.AddWeakLambda(this, [this](TArray<FFGChunkHandle> LoadedChunks)
		{
			if(FG::DebugDrawVoxelRenderDiffs)
			{
				for(int32 Chunk = 0; Chunk < LoadedChunks.Num(); Chunk++)
				{
					UFGVoxelUtils::DebugDrawChunk(GetWorld(), LoadedChunks[Chunk]->ChunkCoordinate, FLinearColor::Green);
				}
			}
		});

		OnRenderCoordinatesAdded.Broadcast(MoveTemp(RenderAdditions));

		AwaitingForcedGeneration = false;
		LastPlayerCoord = PlayerCoord;
	}
}

// Don't create Voxel System in the main menu or on transient levels.
bool UFGVoxelSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	bool ShouldCreate = Super::ShouldCreateSubsystem(Outer);
	ShouldCreate |= Outer->GetName() == TEXT("FrontEnd");
	ShouldCreate |= Outer->GetName() == TEXT("Untitled");
	return ShouldCreate;
}

void UFGVoxelSystem::EnumerateVoxels()
{
	TArray<FPrimaryAssetId> VoxelMetadataAssets;

	UAssetManager::Get().GetPrimaryAssetIdList(
		FPrimaryAssetType("VoxelMetadata"),
		VoxelMetadataAssets,
		EAssetManagerFilter::Default
	);

	if(VoxelMetadataAssets.IsEmpty())
	{
		return;
	}
	
	GVoxelTypeMap.Empty();
	GVoxelTypeFlagMap.Empty();
	
	GVoxelTypeMap.Add(TAG_VOXEL_FG_AIR, VOXELTYPE_NONE);
	GVoxelTypeFlagMap.FindOrAdd(VOXELTYPE_NONE, EFGVoxelFlags::NoFlags);

	int32 VoxelId = 1;
	
	for(FPrimaryAssetId AssetId : VoxelMetadataAssets)
	{
		VoxelId += 1;
		
		TSoftObjectPtr<UFGVoxelMetadata> MetadataAsset = TSoftObjectPtr<UFGVoxelMetadata>(
			UAssetManager::Get().GetPrimaryAssetPath(AssetId));

		if(UFGVoxelMetadata* Metadata = MetadataAsset.LoadSynchronous())
		{
			checkf(!GVoxelTypeMap.Contains(Metadata->VoxelName), TEXT("Duplicate VoxelId found!"));

			GVoxelTypeMap.Add(Metadata->VoxelName, VoxelId);
			GVoxelTypeFlagMap.FindOrAdd(VoxelId, (EFGVoxelFlags)Metadata->Flags);
		}
	}

	VoxelActorManager->EnumerateClassMappings();
}

void UFGVoxelSystem::InitializeRendering()
{
	UpdateRenderDistance(FG::VoxelDefaultRenderDistance);

	if(ActiveMesher.IsSet()) // Destroy existing mesher.
	{
		ActiveMesher.GetValue()->Deinitialize();
		ActiveMesher.GetValue()->Destroy();
		ActiveMesher.Reset();
	}

	UClass* MesherClass = GetDefault<UFGVoxelProjectSettings>()->VoxelMesherClass.LoadSynchronous();

	if(FG::EnableVoxelMeshing && MesherClass)
	{
		// Spawn the mesher.
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |=	RF_Transient;
		SpawnParams.ObjectFlags &= ~RF_Transactional;

		ActiveMesher = GetWorld()->SpawnActor<AFGVoxelMesher>(MesherClass, SpawnParams);
		ActiveMesher.GetValue()->Initialize();
	}
}

void UFGVoxelSystem::FlushRendering()
{
	using namespace FG::Const;
    
	TArray<FIntVector> RemovedChunks;
	Algo::Transform(RenderableHandles, RemovedChunks, [](auto RenderableHandle)
	{
		return RenderableHandle.Value->ChunkCoordinate;
	});

	if(FG::DebugDrawVoxelRenderDiffs)
	{
		for(int32 Chunk = 0; Chunk < RemovedChunks.Num(); Chunk++)
		{
			UFGVoxelUtils::DebugDrawChunk(GetWorld(), RemovedChunks[Chunk], FLinearColor::Red);
		}
	}

	RenderableHandles.Empty();
	OnRenderCoordinatesRemoved.Broadcast(MoveTemp(RemovedChunks));
	
	AwaitingForcedGeneration = true;
}

void UFGVoxelSystem::UpdateRenderDistance(uint32 RenderSizeX)
{
	checkf(RenderSizeX < FG::Const::RenderSizeMax, TEXT("Invalid Render Distance specified!"));
	GRenderSizeX	= RenderSizeX;
	GRenderSizeXYZ	= FMath::Square(RenderSizeX);
	GRenderSizeXYZ	= FMath::Cube(RenderSizeX);
}

void UFGVoxelSystem::DrawDebugChunkData(const FIntVector& ChunkCoordinate)
{
	using namespace FG::Const;

	const FVector ChunkLocation = UFGVoxelUtils::ChunkCoordToVector(ChunkCoordinate);
	FFGChunkHandle ChunkHandle = VoxelGrid->FindChunk(ChunkCoordinate);

	FLinearColor ChunkDebugColor = ChunkHandle.IsValid() ? FLinearColor::White : FLinearColor::Red;

	// Draws red if data wasn't found!
	FG::DebugDrawBox( // Debug draw chunk border.
		GetWorld(),
		ChunkLocation + FVector(ChunkSizeX * VoxelSizeUU / 2),
		FQuat::Identity,
		ChunkSizeX * VoxelSizeUU,
		ChunkDebugColor);

	if(!ChunkHandle.IsValid())
    {
    	return;
    }
	
	FFGVoxelChunk* ChunkData = VoxelGrid->GetChunkDataSafe(ChunkHandle);
	
	for(int32 Voxel = 0; Voxel < ChunkSizeXYZ; Voxel++)
	{
		const FIntVector VoxelCoordinate = UFGVoxelUtils::UnflattenVoxelCoord(Voxel);
		const FVector VoxelLocation = FVector(ChunkLocation) + FVector(VoxelCoordinate * VoxelSizeUU);

		if(ChunkData->GetVoxel(VoxelCoordinate) > VOXELTYPE_NONE) // Opaque check.
		{
			FG::DebugDrawBox( // Debug draw voxel border.
				GetWorld(),
				VoxelLocation + FVector(VoxelSizeUU / 2));
		}
	}
}

void UFGVoxelSystem::ModifyVoxel(FIntVector ChunkCoordinate, FIntVector VoxelCoordinate, int32 NewValue)
{
	FFGChunkHandle ChunkHandle = VoxelGrid->FindChunkChecked(ChunkCoordinate);
	FFGVoxelChunk* ChunkDataPtr = VoxelGrid->GetChunkDataSafe(ChunkHandle);
	
	int32 OldValue = ChunkDataPtr->GetVoxel(VoxelCoordinate);
	ChunkDataPtr->SetVoxel(VoxelCoordinate, NewValue);

	MarkForRemesh(ChunkCoordinate);
	OnVoxelEdited.Broadcast(ChunkCoordinate, VoxelCoordinate, OldValue, NewValue);
}

void UFGVoxelSystem::BatchModifyVoxels(TArray<TPair<FIntVector, FIntVector>> VoxelPositions, int32 NewValue)
{
	TSet<FIntVector> DirtyChunks;
	
	for(auto VoxelPosition : VoxelPositions)
	{
		DirtyChunks.Add(VoxelPosition.Key);
		
		FFGChunkHandle ChunkHandle = VoxelGrid->FindChunkChecked(VoxelPosition.Key);
		FFGVoxelChunk* ChunkDataPtr = VoxelGrid->GetChunkDataSafe(ChunkHandle);
        
        int32 OldValue = ChunkDataPtr->GetVoxel(VoxelPosition.Value);
        ChunkDataPtr->SetVoxel(VoxelPosition.Value, NewValue);
        
        OnVoxelEdited.Broadcast(VoxelPosition.Key, VoxelPosition.Value, OldValue, NewValue);
	}

	for(FIntVector Chunk : DirtyChunks)
	{
		MarkForRemesh(Chunk);
	}
}

void UFGVoxelSystem::MarkForRemesh(const FIntVector& ChunkCoordinate)
{
	PendingRemeshes.Add(ChunkCoordinate);
}
