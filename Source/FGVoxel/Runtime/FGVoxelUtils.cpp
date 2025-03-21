// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelUtils.h"

#include "FGDebugDraw.h"
#include "Containers/FGVoxelChunk.h"
#include "World/FGVoxelSystem.h"

using namespace FG::Const;

namespace FG
{
	static bool EnableVoxelQueryDebug = false;
	FAutoConsoleVariableRef CVarEnableVoxelQueryDebug (
		TEXT("FG.Vox.EnableQueryDebugging"),
		EnableVoxelQueryDebug,
		TEXT("Enables voxel query debugging. (0/1)"),
		ECVF_Default
	);

	static bool DebugDrawDDA = false;
	FAutoConsoleVariableRef CVarDebugDrawDDa (
		TEXT("FG.Vox.DebugDrawDDA"),
		DebugDrawDDA,
		TEXT("Enables debug drawing of DDA intersections. (0/1)"),
		ECVF_Default
	);
}

// @TODO: It honestly doesn't entirely make sense to have this here - make it part of voxel system
// instead probably.
uint32& UFGVoxelUtils::GetNearestVoxelType(UWorld* World, FVector Location)
{
	// @TODO: Fixme;
	static uint32 Air = VOXELTYPE_NONE;
	return Air;
}

TArray<FIntVector2> UFGVoxelUtils::MakeMatrixSpiral2D(uint32 MatrixSizeX, uint32 MatrixSizeY)
{
	TArray<FIntVector2> OutSpiral;
	OutSpiral.Reserve(MatrixSizeX * MatrixSizeY);
	
	int32 Left = 0, Right = MatrixSizeX - 1, Top = 0, Bottom = MatrixSizeY - 1;
	
	while (Left <= Right && Top <= Bottom) {
		// Traverse top row from left to right
		for (int32 Voxel = Left; Voxel <= Right; Voxel++) {
			OutSpiral.Emplace(FIntVector2(Top, Voxel));
		}
		Top++;

		// Traverse rightmost column from top to bottom
		for (int32 Voxel = Top; Voxel <= Bottom; Voxel++) {
			OutSpiral.Emplace(FIntVector2(Voxel, Right));
		}
		Right--;

		// Traverse bottom row from right to left
		if (Top <= Bottom) {
			for (int32 Voxel = Right; Voxel >= Left; Voxel--) {
				OutSpiral.Emplace(FIntVector2(Bottom, Voxel));
			}
			Bottom--;
		}

		// Traverse leftmost column from bottom to top
		if (Left <= Right) {
			for (int32 Voxel = Bottom; Voxel >= Top; Voxel--) {
				OutSpiral.Emplace(FIntVector2(Voxel, Left));
			}
			Left++;
		}
	}
	return OutSpiral;
}

void UFGVoxelUtils::DebugDrawChunk(UWorld* World, FIntVector ChunkCoordinate, FLinearColor Color, double Time)
{
	FG::DebugDrawBox(
		World,
		ChunkCoordToVector(ChunkCoordinate) + FVector((ChunkSizeX * VoxelSizeUU) / 2),
		FQuat::Identity,
		ChunkSizeX * VoxelSizeUU,
		Color,
		Time);
}

int32 UFGVoxelUtils::GetVoxelTypeAtLocation(UWorld* World, const FVector& VoxelLocation)
{
	auto& VoxelGrid = World->GetSubsystem<UFGVoxelSystem>()->VoxelGrid;

	FIntVector ChunkCoordinate = VectorToChunkCoord(VoxelLocation);
	FIntVector VoxelCoordinate = VectorToVoxelCoord(VoxelLocation);

	//DebugDrawChunk(World, ChunkCoordinate, FLinearColor::Green, 0.25);
	
	//FG::DebugDrawBox(
	//	World,
	//	ChunkCoordToVector(ChunkCoordinate) + (FVector(VoxelCoordinate) * VoxelSizeUU) + (VoxelSizeUU / 2.0),
	//	FQuat::Identity,
	//	VoxelSizeUU,
	//	FLinearColor::Blue,
	//	0.25);
	
	FFGChunkHandle ChunkHandle = VoxelGrid->FindChunk(ChunkCoordinate);
	
	if(ChunkHandle.IsValid())
	{
		return VoxelGrid->GetChunkDataSafe(ChunkHandle)->GetVoxel(VoxelCoordinate);
	}
	return VOXELTYPE_NONE;
}

TOptional<FFGVoxelRayHit> UFGVoxelUtils::RayVoxelIntersection(UWorld* World, const FVector Start, const FVector End)
{
    FVector RayDirection = (End - Start).GetSafeNormal();
    
    FVector CurrentPosition = Start;

    FIntVector TargetVoxel(
        FMath::FloorToInt(CurrentPosition.X / VoxelSizeUU),
        FMath::FloorToInt(CurrentPosition.Y / VoxelSizeUU),
        FMath::FloorToInt(CurrentPosition.Z / VoxelSizeUU));

    FIntVector Step(
        RayDirection.X > 0 ? 1 : -1,
        RayDirection.Y > 0 ? 1 : -1,
        RayDirection.Z > 0 ? 1 : -1);

    FVector Delta(
        FMath::Abs(VoxelSizeUU / RayDirection.X),
        FMath::Abs(VoxelSizeUU / RayDirection.Y),
        FMath::Abs(VoxelSizeUU / RayDirection.Z));

    FVector Max(
        (Step.X > 0) ? ((TargetVoxel.X + 1) * VoxelSizeUU - CurrentPosition.X) : (CurrentPosition.X - TargetVoxel.X * VoxelSizeUU),
        (Step.Y > 0) ? ((TargetVoxel.Y + 1) * VoxelSizeUU - CurrentPosition.Y) : (CurrentPosition.Y - TargetVoxel.Y * VoxelSizeUU),
        (Step.Z > 0) ? ((TargetVoxel.Z + 1) * VoxelSizeUU - CurrentPosition.Z) : (CurrentPosition.Z - TargetVoxel.Z * VoxelSizeUU)
    );

    Max.X /= FMath::Abs(RayDirection.X);
    Max.Y /= FMath::Abs(RayDirection.Y);
    Max.Z /= FMath::Abs(RayDirection.Z);

    FVector LastStepDirection;

	auto* VoxSys = World->GetSubsystem<UFGVoxelSystem>();
	
	FIntVector ChunkCoordinate = VectorToChunkCoord(CurrentPosition);
	if (!VoxSys || !VoxSys->VoxelGrid->IsChunkGenerated(ChunkCoordinate))
	{
		return TOptional<FFGVoxelRayHit>();
	}

    while (true)
    {
        if (GetVoxelTypeAtLocation(World, FVector(TargetVoxel) * VoxelSizeUU) > VOXELTYPE_NONE)
        {
            FVector VoxelCenter = FVector(TargetVoxel) * VoxelSizeUU + FVector(VoxelSizeUU / 2.0f);
            return FFGVoxelRayHit(Start, End, VoxelCenter, CurrentPosition, LastStepDirection);
        }

        if (Max.X < Max.Y)
        {
            if (Max.X < Max.Z)
            {
                TargetVoxel.X += Step.X;
                LastStepDirection = FVector(Step.X, 0, 0);
                Max.X += Delta.X;
            }
            else
            {
                TargetVoxel.Z += Step.Z;
                LastStepDirection = FVector(0, 0, Step.Z);
                Max.Z += Delta.Z;
            }
        }
        else
        {
            if (Max.Y < Max.Z)
            {
                TargetVoxel.Y += Step.Y;
                LastStepDirection = FVector(0, Step.Y, 0);
                Max.Y += Delta.Y;
            }
            else
            {
                TargetVoxel.Z += Step.Z;
                LastStepDirection = FVector(0, 0, Step.Z);
                Max.Z += Delta.Z;
            }
        }

        if ((CurrentPosition - Start).Size() > (End - Start).Size())
        {
            break;
        }

        CurrentPosition = FVector(TargetVoxel) * VoxelSizeUU + FVector(VoxelSizeUU / 2.0f);
    }
    return TOptional<FFGVoxelRayHit>();
}

void UFGVoxelUtils::RunCommandOnGameThread(UObject* InUserObject, TUniqueFunction<void()> InFunctor, const TStatId InStatId)
{
	TWeakObjectPtr<> WeakThis = MakeWeakObjectPtr(InUserObject);
	if(!IsInGameThread())
	{
		FFunctionGraphTask::CreateAndDispatchWhenReady(
			[WeakThis, Functor = MoveTemp(InFunctor), InStatId]()
			{
				if(WeakThis.IsValid())
				{
					CSV_SCOPED_TIMING_STAT_EXCLUSIVE(Voxel);
					QUICK_SCOPE_CYCLE_COUNTER(STAT_VoxelUtils_RunCommandOnGameThread);
					FScopeCycleCounter ScopeCycleCounter(InStatId);
					Functor();
				}
			},
			TStatId(),
			nullptr,
			ENamedThreads::GameThread);
	}
	else
	{
		CSV_SCOPED_TIMING_STAT_EXCLUSIVE(Voxel);
		QUICK_SCOPE_CYCLE_COUNTER(STAT_VoxelUtils_RunCommandOnGameThread);
		FScopeCycleCounter ScopeCycleCounter(InStatId);
		InFunctor();
	}
}
