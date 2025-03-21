// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPixelMeshBuilder.h"

namespace FG::PixelMesh
{
	enum class EFGPixelMeshDir : uint8
	{
		Forward = 0,
		Right = 1,
		Back = 2,
		Left = 3,
		Up = 4,
		Down = 5,
		MAX = 6
	};

	static const FIntVector	DirectionMaskTable[6] =
	{
		FIntVector( 1,  0,  0),	// Forward
		FIntVector( 0,  1,  0), // Right
		FIntVector(-1,  0,  0), // Back
		FIntVector( 0, -1,  0), // Left
		FIntVector( 0,  0,  1), // Up
		FIntVector( 0,  0, -1)  // Down
	};

	static const FVector3d BlockVertexTable[8] = {
		FVector3d(1.0, 1.0, 1.0),
		FVector3d(1.0, 0.0, 1.0),
		FVector3d(1.0, 0.0, 0.0),
		FVector3d(1.0, 1.0, 0.0),
		FVector3d(0.0, 0.0, 1.0),
		FVector3d(0.0, 1.0, 1.0),
		FVector3d(0.0, 1.0, 0.0),
		FVector3d(0.0, 0.0, 0.0)
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
}

using namespace FG::PixelMesh;

FFGPixelMesh UFGPixelMeshBuilder::BuildMeshFromTexture(UTexture2D* Texture2D, int32 BlockHeight, float VoxelSizeUU)
{
	if(!Texture2D || BlockHeight <= 0)
	{
		return FFGPixelMesh();
	}

	FFGPixelMesh OutMesh;
	OutMesh.DynamicMesh.EnableVertexUVs(FVector2f::ZeroVector);
	OutMesh.DynamicMesh.EnableVertexNormals(FVector3f::ZeroVector);
	OutMesh.DynamicMesh.EnableVertexColors(FVector3f::ZeroVector);
	OutMesh.DynamicMesh.EnableAttributes();
	OutMesh.DynamicMesh.Attributes()->EnablePrimaryColors();
	OutMesh.UVOverlay = OutMesh.DynamicMesh.Attributes()->PrimaryUV();
	OutMesh.NormalOverlay = OutMesh.DynamicMesh.Attributes()->PrimaryNormals();
	OutMesh.ColorOverlay = OutMesh.DynamicMesh.Attributes()->PrimaryColors();

#if WITH_EDITOR // Editor uncooked data.
	
	TArray64<uint8> EditorData;
	Texture2D->Source.GetMipData(EditorData, 0);
	std::size_t DataSize = EditorData.Num();

	uint8* TextureData = EditorData.GetData();
	
	if(DataSize <= 0)
	{
		return FFGPixelMesh();
	}

	int32 SizeX = Texture2D->Source.GetSizeX();
	int32 SizeY = Texture2D->Source.GetSizeY();
	
#else // !WITH_EDITOR - Cooked Platform Specific Texture.
	
	FTexture2DMipMap& MipMap = Texture2D->GetPlatformData()->Mips[0];
    uint8* TextureData = StaticCast<uint8*>(MipMap.BulkData.Lock(LOCK_READ_ONLY));
	std::size_t DataSize = MipMap.BulkData.GetBulkDataSize();
	
	if(DataSize <= 0)
	{
		return FFGPixelMesh();
	}

    int32 SizeX = MipMap.SizeX;
    int32 SizeY = MipMap.SizeY;
	
#endif // WITH_EDITOR
	
	checkf(TextureData != nullptr, TEXT("Failed to lock texture data."));

	for(int32 VoxelX = 0; VoxelX < SizeX; VoxelX++)
    {
		for(int32 VoxelY = 0; VoxelY < SizeY; VoxelY++)
        {
        	for(int32 VoxelZ = 0; VoxelZ < BlockHeight; VoxelZ++)
        	{
        		FIntVector VoxelCoordinate = FIntVector(VoxelX, VoxelY, VoxelZ);
        		int32 VoxelXYIndex = VoxelY * SizeX + VoxelX;
        		
        		FColor* PixelColor = (FColor*)(TextureData + VoxelXYIndex * 4);

        		if(PixelColor->A == 0) // This voxel is transparent, go next.
        		{
        			continue;
        		}

        		for(int32 Dir = (int32)EFGPixelMeshDir::Forward; Dir < (int32)EFGPixelMeshDir::MAX; Dir++)
        		{
					FIntVector NeighbourCoordinate = VoxelCoordinate + DirectionMaskTable[Dir];

        			if (NeighbourCoordinate.X < 0 || NeighbourCoordinate.X >= SizeX ||
						NeighbourCoordinate.Y < 0 || NeighbourCoordinate.Y >= SizeY ||
						NeighbourCoordinate.Z < 0 || NeighbourCoordinate.Z >= BlockHeight)
        			{
						MakeQuad(OutMesh, VoxelCoordinate, Dir, PixelColor, VoxelSizeUU);
						continue;
					}
        			
        			int32 NeighbourXYIndex = NeighbourCoordinate.Y * SizeX + NeighbourCoordinate.X;
        			FColor* NeighbourColor = (FColor*)(TextureData + NeighbourXYIndex * 4);
        			
					if(NeighbourColor->A == 0) // Neighbour is transparent, make a quad.
					{
						MakeQuad(OutMesh, VoxelCoordinate, Dir, PixelColor, VoxelSizeUU);
					}
        		}
        	}
        }
    }

#if !WITH_EDITOR
	MipMap.BulkData.Unlock();
#endif
	
	return OutMesh;
}

void UFGPixelMeshBuilder::MakeQuad(FFGPixelMesh& PixelMesh, FIntVector VoxelCoordinate, int32 DOF, FColor* Color, float VoxelSizeUU)
{
	for(int32 Vertex = 0; Vertex < 4; Vertex++) // Build Quad.
	{
		const FVector3d Position = FVector3d(VoxelCoordinate) * VoxelSizeUU;

		FVector3d VertexPos = Position + BlockVertexTable[BlockIndexTable[Vertex + DOF * 4]] * VoxelSizeUU;

		PixelMesh.UVOverlay->AppendElement(VertexUVTable[Vertex]);
		PixelMesh.NormalOverlay->AppendElement(FVector3f(DirectionMaskTable[DOF]));
		PixelMesh.ColorOverlay->AppendElement(FVector4f(*Color));
		PixelMesh.DynamicMesh.AppendVertex(FVertexInfo(VertexPos));
	}

	TArray<FIndex3i, TFixedAllocator<2>> Quad {
		FIndex3i(PixelMesh.VertexCount + 3, PixelMesh.VertexCount + 2, PixelMesh.VertexCount),
		FIndex3i(PixelMesh.VertexCount + 2, PixelMesh.VertexCount + 1, PixelMesh.VertexCount)
	};

	for(int32 Tri = 0; Tri < Quad.Num(); Tri++)
	{
		int32 TriIndex = PixelMesh.DynamicMesh.AppendTriangle(Quad[Tri]);
		PixelMesh.UVOverlay->SetTriangle(TriIndex, Quad[Tri]);
		PixelMesh.NormalOverlay->SetTriangle(TriIndex, Quad[Tri]);
		PixelMesh.ColorOverlay->SetTriangle(TriIndex, Quad[Tri]);
	}
	PixelMesh.VertexCount += 4;
}
