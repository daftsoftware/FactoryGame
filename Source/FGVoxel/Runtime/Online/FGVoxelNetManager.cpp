// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelNetManager.h"
#include "FGVoxelUtils.h"
#include "World/FGVoxelSystem.h"
#include "ExtendableGameStateBase.h"
#include "FGDebugDraw.h"
#include "Containers/FGVoxelChunk.h"

namespace FG
{
	static int32 ServerDefaultRelevancyBubbleSize = 8;
	FAutoConsoleVariableRef CVarServerDefaultRelevancyBubbleSize (
		TEXT("FG.ServerRelevancyBubbleSize"),
		ServerDefaultRelevancyBubbleSize,
		TEXT("Overrides the default relevancy bubble size on the server-side."),
		ECVF_Default
	);

	static bool VoxelNetDrawDebug = true;
	FAutoConsoleVariableRef CVarVoxelNetDrawDebug (
		TEXT("FG.VoxelNetDrawDebug"),
		VoxelNetDrawDebug,
		TEXT("Enable debug drawing for voxel networking."),
		ECVF_Default
	);
}

using namespace FG::Const;

void UFGVoxelGameStateSubsystem::BeginPlay()
{
	Super::BeginPlay();

	if(GetWorld()->GetNetMode() != ENetMode::NM_Client) // Server Only.
	{
		// Server has a controller on listen server - insert local PC entry.
		if(APlayerController* PC = GetWorld()->GetFirstPlayerController()) 
		{
			PlayerLastCoords.Add(MakeWeakObjectPtr(PC), TOptional<FIntVector>());
		}
		
		FGameModeEvents::GameModePostLoginEvent.AddWeakLambda(this, [this](AGameModeBase* GM, APlayerController* PC)
		{
			PlayerLastCoords.Add(MakeWeakObjectPtr(PC), TOptional<FIntVector>());
		});
		
		FGameModeEvents::GameModeLogoutEvent.AddWeakLambda(this, [this](AGameModeBase* GM, AController* PC)
		{
			PlayerLastCoords.Remove(CastChecked<APlayerController>(PC));
		});

		GServerStreamingVolumeSizeX		= FG::ServerDefaultRelevancyBubbleSize;
		GServerRelevancyBubbleSizeXY	= FMath::Square(GServerStreamingVolumeSizeX);
		GServerRelevancyBubbleSizeXYZ	= FMath::Cube(GServerStreamingVolumeSizeX);
	}
}

void UFGVoxelGameStateSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(GetWorld()->GetNetMode() != ENetMode::NM_Client) // Server Only.
	{
		auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();

		TSet<FIntVector> IntersectedAdds;
		TSet<FIntVector> IntersectedRemoves;
		
		CalculateChangelists(IntersectedAdds, IntersectedRemoves);

		// Filter unloaded chunks.
		TArray<FIntVector> UnloadedChunks;
		Algo::CopyIf(IntersectedAdds, UnloadedChunks, [&](const FIntVector& ChunkCoord)
		{
			return !VoxSys->VoxelGrid->IsChunkGenerated(ChunkCoord);
		});

		// @TODO: Review this is safe to use these load handles yet, because we use async task
		// to actually make them, so i'm assuming the answer is no.
		
		// Load the new chunks asynchronously.
		FFGVoxelLoadHandle LoadHandle = VoxSys->VoxelGrid->LoadChunkBatchAsync(UnloadedChunks);
		for(int32 Idx = 0; Idx < LoadHandle->GetBatchSize(); Idx++)
		{
			TrackedChunks.Add(LoadHandle->ChunkCoordinates[Idx], LoadHandle->ChunkHandles[Idx]);

			if(FG::VoxelNetDrawDebug)
			{
				FG::DebugDrawBox(
					GetWorld(),
					UFGVoxelUtils::ChunkCoordToVector(LoadHandle->ChunkCoordinates[Idx]),
					FQuat::Identity,
					ChunkSizeX * VoxelSizeUU,
					FLinearColor::Green,
					0.5);
			}
		}

		// Stop referencing chunks that are no longer relevant.
		for(FIntVector& ChunkCoord : IntersectedRemoves)
		{
			TrackedChunks.Remove(ChunkCoord);

			if(FG::VoxelNetDrawDebug)
			{
				FG::DebugDrawBox(
					GetWorld(),
					UFGVoxelUtils::ChunkCoordToVector(ChunkCoord),
					FQuat::Identity,
					ChunkSizeX * VoxelSizeUU,
					FLinearColor::Red,
					0.5);
			}
		}

		for(TMap<FIntVector, TArray<FFGVoxelEdit>>::TIterator It = PendingVoxelEdits.CreateIterator(); It; ++It)
		{
			if(VoxSys->VoxelGrid->IsChunkGenerated(It.Key()))
			{
				for(FFGVoxelEdit& Edit : It.Value())
				{
					VoxSys->ModifyVoxel(Edit.ChunkCoordinate, Edit.VoxelCoordinate, Edit.NewVoxelType);
				}
				It.RemoveCurrent();
			}
		}

		// Dispatch client chunk updates.
		for(TMap<TWeakObjectPtr<APlayerController>, TSet<FIntVector>>::TIterator It = PendingClientUpdates.CreateIterator(); It; ++It)
		{
			FFGChunkUpdatePayload Payload;
			
			if(TSet<FIntVector>::TIterator ChunkIt = It.Value().CreateIterator(); ChunkIt)
			{
				FIntVector ChunkCoord = ChunkIt.ElementIt->Value;
				
				FFGChunkHandle Handle = VoxSys->VoxelGrid->FindChunkChecked(ChunkCoord);
				if(FFGVoxelChunk* ChunkData = VoxSys->VoxelGrid->GetChunkDataUnsafe(Handle))
				{
					Payload.ChunkPositions.Add(ChunkCoord);
					Payload.ChunkData.Add(*ChunkData);
					ChunkIt.RemoveCurrent();
				}
			}

			// @TODO: Fill buffer for the specified client.
			auto* NetMgr = It.Key()->FindComponentByClass<UFGVoxelNetManager>();

			if(It.Value().IsEmpty())
			{
				It.RemoveCurrent();
			}
		}
	}
}

void UFGVoxelGameStateSubsystem::CalculateChangelists(TSet<FIntVector>& IntersectedAdds, TSet<FIntVector>& IntersectedRemoves)
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		checkf(IsValid(PC), TEXT("Player controller was null or pending kill!"));

		if(APawn* Pawn = PC->GetPawn())
		{
			if(!PlayerLastCoords.Contains(MakeWeakObjectPtr(PC))) // No last player coord.
			{
				return;
			}

			const FIntVector PlayerCoord = UFGVoxelUtils::VectorToChunkCoord(Pawn->GetActorLocation());
			const TOptional<FIntVector> LastPlayerCoord = PlayerLastCoords.FindChecked(PC);
			
			PlayerLastCoords.FindChecked(MakeWeakObjectPtr(PC)) = PlayerCoord;

			if(!LastPlayerCoord.IsSet())
			{
				return;
			}
			
			constexpr double ChunkSizeXUU = ChunkSizeX * VoxelSizeUU;
			constexpr double ChunkExtentXUU = ChunkSizeXUU / 2;

			FVector RenderVolumeExtent = FVector(GServerStreamingVolumeSizeX * ChunkSizeX * VoxelSizeUU) / 2;
			FBox LastRenderVolume = FBox::BuildAABB(UFGVoxelUtils::ChunkCoordToVector(LastPlayerCoord.GetValue()), RenderVolumeExtent);
			FBox NewRenderVolume = FBox::BuildAABB(UFGVoxelUtils::ChunkCoordToVector(PlayerCoord), RenderVolumeExtent);

			if(FG::VoxelNetDrawDebug)
			{
#if 0
				FG::DebugDrawBox(
					GetWorld(),
					LastRenderVolume.GetCenter(),
					FQuat::Identity,
					GServerStreamingVolumeSizeX * ChunkSizeX * VoxelSizeUU,
					FLinearColor::Blue,
					1.0);
#endif
			
				FG::DebugDrawBox(
					GetWorld(),
					NewRenderVolume.GetCenter(),
					FQuat::Identity,
					GServerStreamingVolumeSizeX * ChunkSizeX * VoxelSizeUU,
					FLinearColor::White,
					0.5);
			}
			
			TArray<FIntVector> ClientRemovals;
			ClientRemovals.Reserve(GServerRelevancyBubbleSizeXYZ);

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
							ClientRemovals.Add(UFGVoxelUtils::VectorToChunkCoord(RemovalLocation));
						}
					}
				}
			}

			IntersectedRemoves.Append(ClientRemovals);

			TArray<FIntVector> ClientAdditions;
			ClientAdditions.Reserve(GServerRelevancyBubbleSizeXYZ);
		
			// Calculate additions from non-intersecting chunks in new render distance.
			for(double X = NewRenderVolume.Min.X; X < NewRenderVolume.Max.X; X += ChunkSizeXUU)
			{
				for(double Y = NewRenderVolume.Min.Y; Y < NewRenderVolume.Max.Y; Y += ChunkSizeXUU)
				{
					for(double Z = NewRenderVolume.Min.Z; Z < NewRenderVolume.Max.Z; Z += ChunkSizeXUU)
					{
						const FVector RemovalLocation = FVector(X, Y, Z) + ChunkExtentXUU;
		
						if(!FMath::PointBoxIntersection(RemovalLocation, LastRenderVolume))
						{
							const FIntVector ChunkCoordinate = UFGVoxelUtils::VectorToChunkCoord(RemovalLocation);
							ClientAdditions.Add(ChunkCoordinate);
						}
					}
				}
			}

			TSet<FIntVector> ClientUpdateSet;
			Algo::Transform(ClientUpdateSet, ClientAdditions, [](const FIntVector& In) { return In; });
			
			PendingClientUpdates.Add(PC, ClientUpdateSet);
			IntersectedAdds.Append(ClientAdditions);

			for(FIntVector& Add : IntersectedAdds)
			{
				if(IntersectedRemoves.Contains(Add))
				{
					IntersectedRemoves.Remove(Add);
				}
			}
		}
	}
}

UFGVoxelNetManager::UFGVoxelNetManager(const FObjectInitializer& ObjectInitializer)
	: UControllerComponent(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UFGVoxelNetManager::MoveMeToGA_MakeEdit(FIntVector ChunkCoordinate, FIntVector VoxelCoordinate, int32 NewValue)
{
	auto* GS = GetWorld()->GetGameState<AExtendableGameStateBase>();
	auto* VoxGS = GS->GetSubsystem<UFGVoxelGameStateSubsystem>();

	FFGVoxelEdit NewEdit;
	NewEdit.Instigator = GetOwner<APlayerController>();
	NewEdit.ChunkCoordinate = ChunkCoordinate;
	NewEdit.VoxelCoordinate = VoxelCoordinate;
	NewEdit.NewVoxelType = NewValue;
	
	TArray<FFGVoxelEdit>& Queue = VoxGS->PendingVoxelEdits.FindOrAdd(ChunkCoordinate);
	Queue.Add(NewEdit);
}

void UFGVoxelNetManager::ClientReceiveAddressedVoxelUpdate_Implementation(const FFGVoxelNetRef& Address, uint32 NewVoxelType)
{
	auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();

	FIntVector VoxelCoordinate = UFGVoxelUtils::UnflattenVoxelCoord(Address.VoxelIndex);
	VoxSys->ModifyVoxel(Address.ChunkPosWS, VoxelCoordinate, NewVoxelType);
}
