// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "UObject/Object.h"
#include "FGVoxelDefines.h"
#include "FGVoxelUtils.generated.h"

struct FFGVoxelRayHit
{
	FVector Start;
	FVector End;
	FVector VoxelCenter;
	FVector HitLocation;
	FVector StepDirection;

	FVector GetHitFaceNormal()
	{
		if (StepDirection.X > 0)
			return FVector(-1, 0, 0); // Hit the negative X face
		if (StepDirection.X < 0)
			return FVector(1, 0, 0);  // Hit the positive X face

		if (StepDirection.Y > 0)
			return FVector(0, -1, 0); // Hit the negative Y face
		if (StepDirection.Y < 0)
			return FVector(0, 1, 0);  // Hit the positive Y face

		if (StepDirection.Z > 0)
			return FVector(0, 0, -1); // Hit the negative Z face
		if (StepDirection.Z < 0)
			return FVector(0, 0, 1);  // Hit the positive Z face

		return FVector::ZeroVector; // Default case, should never happen	
	}

	FFGVoxelRayHit(FVector InStart, FVector InEnd, FVector InVoxelCenter, FVector InHitLocation, FVector InStepDirection)
		: Start(InStart)
		, End(InEnd)
		, VoxelCenter(InVoxelCenter)
		, HitLocation(InHitLocation)
		, StepDirection(InStepDirection)
	{}
};

/**
 * Factory Game Voxel Utilities Library.
 * Contains various useful conversion and helper functions.
 */
UCLASS(MinimalAPI)
class UFGVoxelUtils : public UObject
{
	GENERATED_BODY()
public:
	
	/**
	 * Convert a vector to a chunk coordinate.
	 * @param Location The input vector.
	 * @return The output chunk coordinate.
	 */
	static FGVOXEL_API FORCEINLINE FIntVector VectorToChunkCoord(FVector Location)
	{
		return FIntVector(
			FMath::FloorToInt(Location.X / (FG::Const::ChunkSizeX * FG::Const::VoxelSizeUU)),
			FMath::FloorToInt(Location.Y / (FG::Const::ChunkSizeX * FG::Const::VoxelSizeUU)),
			FMath::FloorToInt(Location.Z / (FG::Const::ChunkSizeX * FG::Const::VoxelSizeUU))
		);
	}

	static FGVOXEL_API FORCEINLINE FIntVector VectorToVoxelCoord(FVector Location)
	{
		using namespace FG::Const;
		
		FVector GridRelativeLocation = Location - FVector(VectorToChunkCoord(Location) * ChunkSizeX * VoxelSizeUU);
		
		return FIntVector(
			(static_cast<int32>(FMath::FloorToFloat(GridRelativeLocation.X / VoxelSizeUU)) % ChunkSizeX + ChunkSizeX) % ChunkSizeX,
			(static_cast<int32>(FMath::FloorToFloat(GridRelativeLocation.Y / VoxelSizeUU)) % ChunkSizeX + ChunkSizeX) % ChunkSizeX,
			(static_cast<int32>(FMath::FloorToFloat(GridRelativeLocation.Z / VoxelSizeUU)) % ChunkSizeX + ChunkSizeX) % ChunkSizeX
		);
	}

	/**
	 * Convert a chunk coordinate to a vector.
	 * @param ChunkCoordinate The input chunk coordinate.
	 * @return The output vector.
	 */
	static FGVOXEL_API FORCEINLINE FVector ChunkCoordToVector(FIntVector ChunkCoordinate)
	{
		return FVector(ChunkCoordinate * FG::Const::VoxelSizeUU * FG::Const::ChunkSizeX);
	}

	// @TODO: Remove me - confusing - should take chunk coord + vector coord
#if 0
	/**
	 * Convert a voxel coordinate to a local space vector.
	 * @param VoxelCoordinate The input voxel coordinate.
	 * @return The output vector.
	 */
	static FGVOXEL_API FORCEINLINE FVector VoxelCoordToVector(FIntVector VoxelCoordinate)
	{
		return FVector(VoxelCoordinate * FG::Const::VoxelSizeUU);
	}
#endif

	/**
	 * Given a vector, snap it to the nearest voxel.
	 * @param Location The input vector.
	 * @return Input vector snapped to nearest voxel.
	 */
	static FGVOXEL_API FORCEINLINE FVector SnapToNearestVoxel(FVector Location)
	{
		for (int32 Component = 0; Component < 3; Component++)
		{
			Location[Component] = FMath::FloorToFloat(Location[Component] / FG::Const::VoxelSizeUU) * FG::Const::VoxelSizeUU;
		}
		return Location;
	}

	/**
	 * Given a location, get the type of the nearest voxel.
	 * @param World The world to check against.
	 * @param Location The location to check for a voxel type.
	 * @returns VOXELTYPE_NONE if no voxel, or the nearest voxel type.
	 */
	static FGVOXEL_API uint32& GetNearestVoxelType(UWorld* World, FVector Location);

	/**
	 * Given a 2D matrix size, make an inward spiral of FIntVector2s.
	 * @param MatrixSizeX The size of the Matrix on the X Axis.
	 * @param MatrixSizeY The size of the Matrix on the Y Axis.
	 * @return Ordered array of Matrix 2D coordinates in an inward spiral.
	 */
	static FGVOXEL_API TArray<FIntVector2> MakeMatrixSpiral2D(uint32 MatrixSizeX, uint32 MatrixSizeY);

	/**
	 * Flattens a 3D local space chunk coordinate inside the render distance down to a 1D index.
	 * @param ChunkCoordinate The input chunk coordinate to flatten.
	 * @return 1D index of input chunk.
	 */
	static FGVOXEL_API FORCEINLINE int32 FlattenRenderCoord(FIntVector ChunkCoordinate)
	{
		return ChunkCoordinate.Z + (ChunkCoordinate.Y * GRenderSizeX) + (ChunkCoordinate.X * GRenderSizeXY);
	}

	/**
	 * Unflattens 1D chunk index in the render distance to a 3D local space chunk coordinate.
	 * @param Index The input 1D index.
	 * @return Local space chunk coordinate.
	 */
	static FGVOXEL_API FORCEINLINE FIntVector UnflattenRenderCoord(int32 Index)
	{
		return FIntVector(
			Index / GRenderSizeXY,
			(Index / GRenderSizeX) % GRenderSizeX,
			Index % GRenderSizeX
		);
	}
		
	/**
	 * Given a local space voxel coordinate, wrap it asteroids style back around to the other
	 * side of the chunk bounds if it's outside the chunk size.
	 * EG in a 32x32x32 chunk - 32,32,32 would wrap to 0,0,0 and 0,-1,0 would wrap to 0,31,0
	 * @param Voxel position in local space (Out of bounds is allowed).
	 * @return Out wrapped voxel position if applicable.
	 */
	static FGVOXEL_API FORCEINLINE FIntVector WrapVoxelCoord(FIntVector VoxelCoordinate)
	{
		return FIntVector(
			((VoxelCoordinate.X % FG::Const::ChunkSizeX) + FG::Const::ChunkSizeX) % FG::Const::ChunkSizeX,
			((VoxelCoordinate.Y % FG::Const::ChunkSizeX) + FG::Const::ChunkSizeX) % FG::Const::ChunkSizeX,
			((VoxelCoordinate.Z % FG::Const::ChunkSizeX) + FG::Const::ChunkSizeX) % FG::Const::ChunkSizeX
		);
	}

	/**
	 * Flatten a local space voxel coordinate down to a 1D index.
	 * @param Local space voxel coordinate to flatten. (relative to chunk bounds)
	 * @return Flattened local space voxel position.
	 */
	static FGVOXEL_API FORCEINLINE int32 FlattenVoxelCoord(const FIntVector& VoxelCoordinate)
	{
		return VoxelCoordinate.Z + (VoxelCoordinate.Y * FG::Const::ChunkSizeX) + (VoxelCoordinate.X * FG::Const::ChunkSizeXY);
	}

	static FGVOXEL_API FORCEINLINE int32 FlattenVoxelCoord2D(const FIntVector2& VoxelCoordinate2D)
	{
		return VoxelCoordinate2D.Y * FG::Const::ChunkSizeX + VoxelCoordinate2D.X;
	}

	/**
	 * Unflatten a 1D index back to a local space voxel coordinate.
	 * @param Voxel 1D index to unflatten.
	 * @return Unflattened local space voxel position (relative to chunk bounds)
	 */
	static FGVOXEL_API FORCEINLINE FIntVector UnflattenVoxelCoord(int32 Index)
	{
		return FIntVector(
			Index / FG::Const::ChunkSizeXY,
			(Index / FG::Const::ChunkSizeX) % FG::Const::ChunkSizeX,
			Index % FG::Const::ChunkSizeX
		);
	}

	/**
	 * Unflatten a 1D index back to a local space voxel coordinate.
	 * @param Index Voxel 1D index to unflatten.
	 * @param OutChunkCoordinate Unflattened local space voxel position (relative to chunk bounds)
	 */
	static FGVOXEL_API FORCEINLINE void UnflattenVoxelCoordFast(int32 Index, FIntVector& OutChunkCoordinate)
	{
		OutChunkCoordinate.X = Index >> 10;				// Index / 1024
		OutChunkCoordinate.Y = (Index & 0x3FF) >> 5;	// (Index % 1024) / 32
		OutChunkCoordinate.Z = Index & 0x1F;			// Index % 32
	}

	/**
    * Unflatten a 1D index back to a local space voxel coordinate 2D.
    * @param Voxel 1D index to unflatten.
    * @return Unflattened local space voxel position XY (relative to chunk bounds)
    */
	static FGVOXEL_API FORCEINLINE FIntPoint UnflattenVoxelCoord2D(int32 Index)
	{
		return FIntPoint(
			Index % FG::Const::ChunkSizeX,
			Index / FG::Const::ChunkSizeX
		);
	}

	static FGVOXEL_API void DebugDrawChunk(UWorld* World, FIntVector ChunkCoordinate, FLinearColor Color = FLinearColor::Red, double Time = 0.25);

	/**
	 * Given a location, get the type of the nearest voxel.
	 * @param World The world to check against.
	 * @param VoxelLocation The location to check for a voxel type.
	 * @returns VOXELTYPE_NONE if invalid, or air.
	 */
	static FGVOXEL_API int32 GetVoxelTypeAtLocation(UWorld* World, const FVector& VoxelLocation);

	/**
	 * Raycast traversal into a voxel grid using DDA algorithm.
	 * @param World The world to check against.
	 * @param Start The start of the ray.
	 * @param End The end of the ray.
	 * @return Voxel location of the first opaque voxel hit.
	 */
	static FGVOXEL_API TOptional<FFGVoxelRayHit> RayVoxelIntersection(UWorld* World, const FVector Start, const FVector End);

	static FGVOXEL_API TOptional<FVector> QuantizeRayToVoxelNormal(FVector VoxelLocation, FVector Start, FVector End);
	
	static FGVOXEL_API void RunCommandOnGameThread(UObject* InUserObject, TUniqueFunction<void()> InFunctor, TStatId InStatId);
};
