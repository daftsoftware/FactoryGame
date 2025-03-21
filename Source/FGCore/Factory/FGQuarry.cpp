// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGQuarry.h"
#include "World/FGVoxelSystem.h"
#include "FGVoxelUtils.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/StreamableManager.h"
#include "Logging/StructuredLog.h"
#include "System/FGAssetManager.h"

/**
 * @TODO:
 * When a quarry is doing it's thang on a loaded chunk it's going to try to remesh the chunk
 * even if it's too far away for us to care or see it. So we shouldn't remesh a chunk unless
 * it's within the render distance of the player.
 */

/**
 * @TODO:
 * We need an invunerable flag on voxels so that we can mark the parts of the quarry that
 * aren't the actual quarry block as not being able to be mined. The quarry will essentially place
 * invunerable non-opaque voxels around the quarry region, and then just use an instanced static mesh
 * for the visuals of the quarry.
 */

AFGQuarry::AFGQuarry()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	QuarryFrameMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("QuarryFrameMesh"));
	QuarryFrameMesh->SetupAttachment(RootComponent);

	QuarryHeadSupportMeshX = CreateDefaultSubobject<USplineMeshComponent>(TEXT("QuarryHeadSupportMeshX"));
	QuarryHeadSupportMeshX->SetupAttachment(RootComponent);
	QuarryHeadSupportMeshX->SetUsingAbsoluteLocation(true);
	QuarryHeadSupportMeshX->SetUsingAbsoluteRotation(true);
	QuarryHeadSupportMeshX->SetUsingAbsoluteScale(true);

	QuarryHeadSupportMeshY = CreateDefaultSubobject<USplineMeshComponent>(TEXT("QuarryHeadSupportMeshY"));
	QuarryHeadSupportMeshY->SetupAttachment(RootComponent);
	QuarryHeadSupportMeshY->SetUsingAbsoluteLocation(true);
	QuarryHeadSupportMeshY->SetUsingAbsoluteRotation(true);
	QuarryHeadSupportMeshY->SetUsingAbsoluteScale(true);
	
	QuarryDrillBitMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("QuarryDrillBitMesh"));
	QuarryDrillBitMesh->SetupAttachment(RootComponent);
	QuarryDrillBitMesh->SetUsingAbsoluteLocation(true);
	QuarryDrillBitMesh->SetUsingAbsoluteRotation(true);
	QuarryDrillBitMesh->SetUsingAbsoluteScale(true);
}

void AFGQuarry::BeginPlay()
{
	Super::BeginPlay();
	
	TSoftObjectPtr<UStaticMesh> MeshAsset = QuarryFrameMeshAsset;
	FStreamableManager& StreamableMgr = UFGAssetManager::GetStreamableManager();

	StreamableMgr.RequestAsyncLoad(MeshAsset.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
		[this, MeshAsset]()
	{
		check(IsInGameThread());
		QuarryFrameMesh->SetStaticMesh(MeshAsset.Get());
		QuarryFrameMesh->SetNumCustomDataFloats(7);

		SetupFrame();
	}));

	FVector QuarryBlockLocation = GetActorLocation() - FVector(FG::Const::VoxelSizeUU / 2);
	CurrentHeadLocation = QuarryBlockLocation;

	// @TODO: After setup, add FG.Voxel.QuarryReserved blocks around the quarry region.
	// We will use these to prevent the quarry frame from being tampered with by players,
	// eg if a player places blocks inside the frame itself.
}

void AFGQuarry::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();
	
	FVector QuarryBlockLocation = GetActorLocation() - FVector(FG::Const::VoxelSizeUU / 2);
	FVector QuarryRegionStartLocation = (QuarryBlockLocation + (FVector::UpVector * DefaultRegion.Max.Z * FG::Const::VoxelSizeUU));
	FVector RegionBoxOffset = (GetActorForwardVector() * FG::Const::VoxelSizeUU) + (FVector(DefaultRegion.Max * FG::Const::VoxelSizeUU) / 2);

	TArray<TPair<FIntVector, FIntVector>> VoxelsToDig;

	bool WithinDistanceForDig = false;

	CurrentHeadLocation = FMath::VInterpConstantTo(CurrentHeadLocation, TargetHeadLocation, DeltaSeconds, 1000.0f);

	// Is the head in distance to dig.
	if((CurrentHeadLocation - TargetHeadLocation).Size2D() < 1.0)
	{
		WithinDistanceForDig = true;
	}

	DigCooldown -= DeltaSeconds;
	if(DigCooldown <= 0.0)
	{
		float SubstepDelta = DigRateSeconds - DigCooldown;
		int32 Substeps = FMath::FloorToInt(SubstepDelta / DigRateSeconds);
		float RefundedTime = SubstepDelta - (Substeps * DigRateSeconds);
		DigCooldown += DigRateSeconds - RefundedTime;

		VoxelsToDig.Reserve(Substeps);

		for(int32 Substep = 0; Substep < Substeps; Substep++)
		{
			DrawDebugBox(
				GetWorld(),
				QuarryBlockLocation + RegionBoxOffset,
				FVector(DefaultRegion.Max / 2) * FG::Const::VoxelSizeUU,
				FColor::Green,
				false,
				1.0f);

			FVector HeadLocation = QuarryRegionStartLocation + FVector(HeadOffset * FG::Const::VoxelSizeUU);
			TargetHeadLocation = HeadLocation;
			int32 TargetVoxelType = UFGVoxelUtils::GetVoxelTypeAtLocation(GetWorld(), HeadLocation);

			if(WithinDistanceForDig)
            {
				if(TargetVoxelType > VOXELTYPE_NONE)
				{
					DrawDebugBox(
						GetWorld(),
						HeadLocation + FVector(FG::Const::VoxelSizeUU / 2),
						FVector(FG::Const::VoxelSizeUU / 2),
						FColor::Green,
						false,
						1.0f);

					FIntVector TargetChunkCoordinate = UFGVoxelUtils::VectorToChunkCoord(HeadLocation);
					FIntVector TargetVoxelCoordinate = UFGVoxelUtils::VectorToVoxelCoord(HeadLocation);

					VoxelsToDig.Emplace(TargetChunkCoordinate, TargetVoxelCoordinate);
				}
				else
				{
					DrawDebugBox(
						GetWorld(),
						HeadLocation + FVector(FG::Const::VoxelSizeUU / 2),
						FVector(FG::Const::VoxelSizeUU / 2),
						FColor::Red,
						false,
						1.0f);
				}

				FIntVector NewOffset = FIntVector::ZeroValue;
				NewOffset.X += 1;
		
				if(HeadOffset.X >= DefaultRegion.Max.X)
				{
					HeadOffset.X = 0;
					NewOffset.Y += 1;
				}
		
				if(HeadOffset.Y >= DefaultRegion.Max.Y)
				{
					HeadOffset.Y = 0;
					NewOffset.Z -= 1;
				}

				if(HeadOffset.Z >= DefaultRegion.Max.Z)
				{
					HeadOffset.Z = 0;
				}

				if(HeadOffset.Z <= -MaximumDepth)
				{
					return;
				}

				HeadOffset += NewOffset;
			}
		}
	}

	FVector DrillBitPosition = CurrentHeadLocation;
	DrillBitPosition.Z = QuarryBlockLocation.Z + (DefaultRegion.Max.Z * FG::Const::VoxelSizeUU);
	
	QuarryDrillBitMesh->SetSplineUpDir(FVector::RightVector);
	
	// Set drillbit XY and then Z from region top to the current depth
	QuarryDrillBitMesh->SetStartAndEnd(
		DrillBitPosition,
		-FVector::UpVector,
		CurrentHeadLocation,
		-FVector::UpVector);

	float TextureULength = (QuarryRegionStartLocation.Z - CurrentHeadLocation.Z) / FG::Const::VoxelSizeUU;

	UE_LOGFMT(LogTemp, Display, "TextureULength: {Length}", TextureULength);

	QuarryDrillBitMesh->SetDefaultCustomPrimitiveDataVector3(0, // U
		FVector(TextureULength, TextureULength, 1));

	QuarryDrillBitMesh->SetDefaultCustomPrimitiveDataVector3(3, // V
		FVector(1, 1, 1));

	// Support with forward and backwards motion (facing horizontally on X)
	FVector SupportXPosition = QuarryBlockLocation;
	SupportXPosition.X = CurrentHeadLocation.X;
	SupportXPosition.Z = DrillBitPosition.Z;
    
    // Set support X
    QuarryHeadSupportMeshX->SetStartAndEnd(
    	SupportXPosition,
    	FVector::RightVector,
    	SupportXPosition + (FVector::RightVector * DefaultRegion.Max.X * FG::Const::VoxelSizeUU),
    	FVector::RightVector);

	QuarryHeadSupportMeshX->SetDefaultCustomPrimitiveDataVector3(0, // U
		FVector(DefaultRegion.Max.X, 1, DefaultRegion.Max.X));

	QuarryHeadSupportMeshX->SetDefaultCustomPrimitiveDataVector3(3, // V
		FVector(1, 1, 1));

	// Support with left and right motion (facing horizontally on Y)
    FVector SupportYPosition = QuarryBlockLocation;
	SupportYPosition.Y = CurrentHeadLocation.Y;
	SupportYPosition.Z = DrillBitPosition.Z;

	QuarryHeadSupportMeshY->SetDefaultCustomPrimitiveDataVector3(0, // U
		FVector(1, DefaultRegion.Max.Y, DefaultRegion.Max.Y));

	QuarryHeadSupportMeshY->SetDefaultCustomPrimitiveDataVector3(3, // V
		FVector(1, 1, 1));
	
    // Set support Y
    QuarryHeadSupportMeshY->SetStartAndEnd(
    	SupportYPosition,
    	FVector::ForwardVector,
    	SupportYPosition + (FVector::ForwardVector * DefaultRegion.Max.Y * FG::Const::VoxelSizeUU),
    	FVector::ForwardVector);

	VoxSys->BatchModifyVoxels(VoxelsToDig, VOXELTYPE_NONE);
}

void AFGQuarry::SetupFrame()
{
	FVector FrameMin = GetActorLocation() + (FVector::ForwardVector * FG::Const::VoxelSizeUU);

	TArray<float> CustomData;
	CustomData.SetNum(7);
	std::fill_n(CustomData.GetData(), 7, 1.0f);

	UE_LOGFMT(LogTemp, Display, "NumIndexes: {Indexes}", QuarryFrameMesh->GetNumInstances());

	TArray<FVector> FrameSegmentLocations;
		
	FBox RegionBox(
		FrameMin + FVector(DefaultRegion.Min * FG::Const::VoxelSizeUU),
		FrameMin + FVector(DefaultRegion.Max * FG::Const::VoxelSizeUU)
	);
		
	FVector Vertices[8];
	RegionBox.GetVertices(Vertices);

	static constexpr int32 EdgeLUT[12][2] = {
		{0, 1}, {0, 2}, {0, 3}, // From Min corner to three adjacent corners
		{7, 4}, {7, 5}, {7, 6}, // From Max corner to three adjacent corners
		{1, 5}, {1, 6},         // Edges connected to (Min.X, Min.Y, Max.Z)
		{2, 4}, {2, 6},         // Edges connected to (Min.X, Max.Y, Min.Z)
		{3, 4}, {3, 5}          // Edges connected to (Max.X, Min.Y, Min.Z)
	};
	
	for (int32 Edge = 0; Edge < 12; Edge++)
	{
		FVector Start = Vertices[EdgeLUT[Edge][0]];
		FVector End = Vertices[EdgeLUT[Edge][1]];
		FVector Direction = (End - Start).GetSafeNormal();
		float EdgeLength = (End - Start).Size();
		int32 NumSteps = FMath::CeilToInt(EdgeLength / FG::Const::VoxelSizeUU);

		FrameSegmentLocations.Reserve(NumSteps);
	
		for (int32 Step = 0; Step <= NumSteps; Step++)
		{
			FVector Point = Start + Direction * FG::Const::VoxelSizeUU * Step;
			if ((Point - Start).Size() <= EdgeLength)
			{
				FrameSegmentLocations.Add(Point);
			}
		}
	}

	for(const FVector& SegmentLocation : FrameSegmentLocations)
	{
		QuarryFrameMesh->AddInstance(FTransform(FRotator::ZeroRotator, SegmentLocation, FVector::OneVector), true);
		//QuarryFrameMesh->SetCustomData(0, CustomData, true);
	}
}