// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelInstancedChunkMesh.h"
#include "FGDebugDraw.h"
#include "FGVoxelDefines.h"
#include "FGVoxelUtils.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Misc/FGVoxelProjectSettings.h"
#include "Containers/FGVoxelGrid.h"
#include "World/FGVoxelSystem.h"

using namespace FG::Const;

static const FIntVector	DOFMaskTable[6] = {
	{  0,  0,  1 },
	{  0,  0, -1 },
	{  0,  1,  0 },
	{  0, -1,  0 },
	{  1,  0,  0 },
	{ -1,  0,  0 }
};

AFGVoxelInstancedChunkMesh::AFGVoxelInstancedChunkMesh()
{
	PrimaryActorTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
	bListedInSceneOutliner = false;
#endif

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	
	ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>("ISM");
	ISM->SetupAttachment(RootComponent);
	ISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ISM->SetCanEverAffectNavigation(false);
	ISM->SetAffectDistanceFieldLighting(false);
	ISM->SetVisibleInRayTracing(false);
}

void AFGVoxelInstancedChunkMesh::PostInitProperties()
{
	Super::PostInitProperties();

	if(IsTemplate())
	{
		return;
	}
	
	UFGVoxelProjectSettings* VoxelSettings = GetMutableDefault<UFGVoxelProjectSettings>();
    UStaticMesh* VoxelMesh = VoxelSettings->VoxelISMMesh.LoadSynchronous();
    checkf(VoxelMesh, TEXT("Voxel mesh not set in project settings!"));
    
    ISM->SetStaticMesh(VoxelMesh);
}

void AFGVoxelInstancedChunkMesh::GenerateMesh()
{
	using namespace FG::Const;

	FG::DebugDrawBox(
		GetWorld(),
		GetActorLocation(),
		FQuat::Identity,
		ChunkSizeX * VoxelSizeUU,
		FLinearColor::Blue,
		0.5);

	auto& VoxelGrid = GetWorld()->GetSubsystem<UFGVoxelSystem>()->VoxelGrid;
	FFGVoxelChunk& ChunkData = *VoxelGrid->GetChunkDataSafe(ChunkIndex);
	
	for(int32 Voxel = 0; Voxel < ChunkSizeXYZ; Voxel++)
	{
		FIntVector VoxelCoordinate = UFGVoxelUtils::UnflattenVoxelCoord(Voxel);

		if(ChunkData.GetVoxel(VoxelCoordinate) > VOXELTYPE_NONE) // Non-opqaue, paint
        {
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(FVector(VoxelCoordinate) * VoxelSizeUU);
			InstanceTransform.SetScale3D(FVector(VoxelSizeUU));
			ISM->AddInstance(InstanceTransform);
        }
		
#if 0
		if(ChunkData.GetVoxel(VoxelCoordinate) == VOXELTYPE_NONE) // Non-opqaue, skip.
		{
			continue;
		}
		
		bool NeighbouringAir = false;

		for(int32 DOF = 0; DOF < 6; DOF++)
		{
			const FIntVector Neighbour = VoxelCoordinate + DOFMaskTable[DOF];

			// Out of bounds check. @TODO: We can skip this check by oversizing the data with a zeroed margin.
			if(Neighbour.X >= ChunkSizeX || Neighbour.Y >= ChunkSizeX || Neighbour.Z >= ChunkSizeX ||
				Neighbour.X < 0.0 || Neighbour.Y < 0.0 || Neighbour.Z < 0.0)
			{
				continue;
			}

			if(ChunkData.GetVoxel(Neighbour) == VOXELTYPE_NONE) // Check for Air.
			{
				NeighbouringAir |= true;
			}
		}

		if (NeighbouringAir)
		{
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(FVector(VoxelCoordinate) * VoxelSizeUU);
			InstanceTransform.SetScale3D(FVector(VoxelSizeUU));
			ISM->AddInstance(InstanceTransform);
		}
#endif
	}
}

void AFGVoxelInstancedChunkMesh::ClearMesh()
{
	ISM->ClearInstances();
}
