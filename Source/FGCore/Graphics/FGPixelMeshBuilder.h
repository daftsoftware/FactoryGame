// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "Subsystems/EngineSubsystem.h"
#include "FGPixelMeshBuilder.generated.h"

using namespace UE::Geometry;

struct FFGPixelMesh
{
    FDynamicMesh3 DynamicMesh;
	FDynamicMeshUVOverlay* UVOverlay;
    FDynamicMeshNormalOverlay* NormalOverlay;
	FDynamicMeshColorOverlay* ColorOverlay;
	int32 VertexCount = 0;
	
	FFGPixelMesh() = default;
};

UCLASS()
class FGCORE_API UFGPixelMeshBuilder final : public UEngineSubsystem
{
	GENERATED_BODY()
public:

	static ThisClass* Get()
	{
		return GEngine->GetEngineSubsystem<ThisClass>();
	}

	FFGPixelMesh BuildMeshFromTexture(UTexture2D* Texture2D, int32 BlockHeight = 2, float VoxelSizeUU = 50.0);
	
private:
	
	void MakeQuad(FFGPixelMesh& PixelMesh, FIntVector VoxelCoordinate, int32 DOF, FColor* Color, float VoxelSizeUU);
};
