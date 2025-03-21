// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelSimpleChunkMesh.h"
#include "FGVoxelDefines.h"
#include "FGVoxelUtils.h"
#include "Containers/FGVoxelChunk.h"
#include "Misc/FGVoxelProjectSettings.h"
#include "Containers/FGVoxelGrid.h"
#include "World/FGVoxelSystem.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicMesh/MeshAttributeUtil.h"
#include "Logging/StructuredLog.h"

using namespace FG::Const;

static const FIntVector	DOFMaskTable[6] = 
{
	FIntVector( 1,  0,  0),	// Forward
	FIntVector( 0,  1,  0), // Right
	FIntVector(-1,  0,  0), // Back
	FIntVector( 0, -1,  0), // Left
	FIntVector( 0,  0,  1), // Up
	FIntVector( 0,  0, -1)  // Down
};

static const FVector3d BlockVertexTable[8] = {
	FVector3d(VoxelSizeUU,    VoxelSizeUU,	VoxelSizeUU),
	FVector3d(VoxelSizeUU,    0.0,			VoxelSizeUU),
	FVector3d(VoxelSizeUU,    0.0,			0.0),
	FVector3d(VoxelSizeUU,    VoxelSizeUU,	0.0),
	FVector3d(0.0,            0.0,			VoxelSizeUU),
	FVector3d(0.0,            VoxelSizeUU,	VoxelSizeUU),
	FVector3d(0.0,            VoxelSizeUU,	0.0),
	FVector3d(0.0,            0.0,			0.0)
};

static const int32 BlockIndexTable[24] = {
	0,  1,  2,  3, // Forward
	5,  0,  3,  6, // Right
	4,  5,  6,  7, // Back
	1,  4,  7,  2, // Left
	5,  4,  1,  0, // Up
	3,  2,  7,  6  // Down
};

static const FVector2f VertexUVTable[4] = {
	FVector2f(0.0f, 0.0f),  // Bottom-left
	FVector2f(1.0f, 0.0f),  // Bottom-right
	FVector2f(1.0f, 1.0f),  // Top-right
	FVector2f(0.0f, 1.0f)   // Top-left
};

namespace FG
{
	bool MesherWireframeMode = false;
	FAutoConsoleVariableRef CVarMesherWireframeMode(
		TEXT("FG.Mesher.WireframeMode"),
		MesherWireframeMode,
		TEXT("Enables wireframe mode for the mesher."),
		ECVF_Default
	);

	int32 MesherLODOverride = 1;
	FAutoConsoleVariableRef CVarMesherLODOverride(
		TEXT("FG.Mesher.LODOverride"),
		MesherLODOverride,
		TEXT("Explicitly overrides the LOD used on the Mesher."),
		ECVF_Default
	);
}

AFGVoxelSimpleChunkMesh::AFGVoxelSimpleChunkMesh()
	: VertexCount(0)
{
	PrimaryActorTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
	bListedInSceneOutliner = false;
#endif

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	// @TODO: MeshComponent Location is wrong! Should be offset by half chunk size UU.
	
	DynMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>("DynMeshComponent");
	DynMeshComponent->SetupAttachment(RootComponent);
	DynMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFGVoxelSimpleChunkMesh::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(IsTemplate())
	{
		return;
	}

	const UFGVoxelProjectSettings* VoxelSettings = GetDefault<UFGVoxelProjectSettings>();
	
	if(UMaterialInterface* VoxelUberShader = VoxelSettings->VoxelUberShader.LoadSynchronous())
	{
		DynMeshComponent->ConfigureMaterialSet({ VoxelUberShader });
	}
}

void AFGVoxelSimpleChunkMesh::GenerateMesh()
{
	auto& VoxelGrid = GetWorld()->GetSubsystem<UFGVoxelSystem>()->VoxelGrid;
	FFGVoxelChunk& ChunkData = *VoxelGrid->GetChunkDataUnsafe(ChunkHandle);

	MeshLOD = (EFGVoxelMeshLOD)(FMath::RoundUpToPowerOfTwo(FG::MesherLODOverride));

	int32 MesherSizeX = ChunkSizeX / (int32)MeshLOD;
	int32 MesherSizeXY = ChunkSizeXY / (int32)MeshLOD;
	int32 MesherSizeXYZ = ChunkSizeXYZ / (int32)MeshLOD;
	
	int32 NeighbourOffsets[6] = {
    	MesherSizeXY,	    // Forward
    	MesherSizeX,		// Right
    	-MesherSizeXY,	    // Back
    	-MesherSizeX,	    // Left
    	1,				    // Up
    	-1				    // Down
    };
	
	TArray<int32> VoxelTypes = ChunkData.GetVoxelTypesInChunk();
	
	TMap<int32, EFGVoxelFlags> VoxelFlags;
	VoxelFlags.Reserve(VoxelTypes.Num());

	for(int32 VoxelType : VoxelTypes)
	{
		VoxelFlags.Add(VoxelType, GetFlagsForVoxelType(VoxelType));
	}

	TArray<bool> OpaqueVoxels;
	OpaqueVoxels.SetNum(MesherSizeXYZ);
	bool* RESTRICT OpaqueVoxelsPtr = OpaqueVoxels.GetData();
	
	for(int32 Voxel = 0; Voxel < MesherSizeXYZ; Voxel++)
	{
		int32 VoxelType = ChunkData[Voxel * (int32)MeshLOD];
		EFGVoxelFlags Flags = VoxelFlags[VoxelType];
		OpaqueVoxelsPtr[Voxel] = EnumHasAnyFlags(Flags, EFGVoxelFlags::Opaque);
	}

	const auto* VoxelSettings = GetDefault<UFGVoxelProjectSettings>();
	DynMeshComponent->SetMaterial(0, VoxelSettings->VoxelUberShader.LoadSynchronous());
	
	FDynamicMesh3 ChunkMesh;
	ChunkMesh.EnableVertexUVs(FVector2f::ZeroVector);
	ChunkMesh.EnableVertexNormals(FVector3f::ZeroVector);
	ChunkMesh.EnableVertexColors(FVector3f::ZeroVector);
	ChunkMesh.EnableAttributes();
	ChunkMesh.Attributes()->EnablePrimaryColors();

	UVOverlay = ChunkMesh.Attributes()->PrimaryUV();
	NormalOverlay = ChunkMesh.Attributes()->PrimaryNormals();

	if(FG::MesherWireframeMode)
	{
		DynMeshComponent->SetEnableWireframeRenderPass(true);
	}

	FIntVector VoxelCoordinate;
	
	for(int32 Voxel = 0; Voxel < MesherSizeXYZ; Voxel++)
	{
		UFGVoxelUtils::UnflattenVoxelCoordFast(Voxel * (int32)MeshLOD, VoxelCoordinate);

		if(!OpaqueVoxelsPtr[Voxel]) // We are transparent, skip.
		{
			continue;
		}

		for(int32 DOF = 0; DOF < 6; DOF++)
		{
			int32 NeighbourIndex = Voxel + NeighbourOffsets[DOF];

			if(NeighbourIndex >= 0 && NeighbourIndex < MesherSizeXYZ)
			{
				if(!OpaqueVoxelsPtr[NeighbourIndex]) // Neighbouring a transparent voxel.
				{
					MakeQuad(ChunkMesh, VoxelCoordinate, DOF);
				}
			}
		}
	}

	if(ChunkMesh.TriangleCount() > 0)
	{
		DynMeshComponent->SetMesh(MoveTemp(ChunkMesh));

		DynMeshComponent->FastNotifyVertexAttributesUpdated(EMeshRenderAttributeFlags::VertexUVs);
		DynMeshComponent->FastNotifyVertexAttributesUpdated(EMeshRenderAttributeFlags::VertexNormals);
		DynMeshComponent->NotifyMeshUpdated();
	}
}

void AFGVoxelSimpleChunkMesh::ClearMesh()
{
	// Clear mesh
	DynMeshComponent->SetMesh(FDynamicMesh3());
	DynMeshComponent->NotifyMeshUpdated();
	VertexCount = 0;
}

void AFGVoxelSimpleChunkMesh::MakeQuad(FDynamicMesh3& DynMesh, FIntVector VoxelCoordinate, int32 DOF)
{
	for(int32 Vertex = 0; Vertex < 4; Vertex++) // Build Quad.
	{
		const FVector3d Position = FVector3d(VoxelCoordinate) * VoxelSizeUU;

		FVector3d VertexPos = Position + (BlockVertexTable[BlockIndexTable[Vertex + DOF * 4]] * (int32)MeshLOD);

		UVOverlay->AppendElement(VertexUVTable[Vertex]);
		NormalOverlay->AppendElement(FVector3f(DOFMaskTable[DOF]));
		DynMesh.AppendVertex(FVertexInfo(VertexPos));
	}

	TArray<FIndex3i, TFixedAllocator<2>> Quad {
		FIndex3i(VertexCount + 3, VertexCount + 2, VertexCount),
		FIndex3i(VertexCount + 2, VertexCount + 1, VertexCount)
	};

	for(int32 Tri = 0; Tri < Quad.Num(); Tri++)
	{
		int32 TriIndex = DynMesh.AppendTriangle(Quad[Tri]);
		UVOverlay->SetTriangle(TriIndex, Quad[Tri]);
		NormalOverlay->SetTriangle(TriIndex, Quad[Tri]);
	}
	VertexCount += 4;
}
