// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPixelMeshDebugger.h"
#include "FGPixelMeshBuilder.h"
#include "Components/DynamicMeshComponent.h"
#include "Misc/FGVoxelProjectSettings.h"

AFGPixelMeshDebugger::AFGPixelMeshDebugger()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	DynMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynMeshComponent"));
	DynMeshComponent->SetupAttachment(RootComponent);
}

void AFGPixelMeshDebugger::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(IsTemplate())
	{
		return;
	}

	const UFGVoxelProjectSettings* VoxelSettings = GetDefault<UFGVoxelProjectSettings>();
	
	if(UMaterialInterface* PixelMeshShader = VoxelSettings->PixelMeshShader.LoadSynchronous())
	{
		DynMeshComponent->SetMaterial(0, PixelMeshShader);
	}
	
	FFGPixelMesh PixelMesh = UFGPixelMeshBuilder::Get()->BuildMeshFromTexture(Texture2D, 2);
	
	if(PixelMesh.DynamicMesh.TriangleCount() > 0)
	{
		DynMeshComponent->SetMesh(MoveTemp(PixelMesh.DynamicMesh));
		
		DynMeshComponent->FastNotifyVertexAttributesUpdated(EMeshRenderAttributeFlags::VertexUVs);
		DynMeshComponent->FastNotifyVertexAttributesUpdated(EMeshRenderAttributeFlags::VertexNormals);
		DynMeshComponent->NotifyMeshUpdated();
	}
}

#if WITH_EDITOR

void AFGPixelMeshDebugger::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AFGPixelMeshDebugger, Texture2D))
	{
		const UFGVoxelProjectSettings* VoxelSettings = GetDefault<UFGVoxelProjectSettings>();
		
		if(UMaterialInterface* PixelMeshShader = VoxelSettings->PixelMeshShader.LoadSynchronous())
		{
			DynMeshComponent->SetMaterial(0, PixelMeshShader);
		}

		static constexpr float MiniVoxelSizeUU = 50.f;
		float SizeX = Texture2D->GetSizeX() * MiniVoxelSizeUU;
		float SizeY = Texture2D->GetSizeY() * MiniVoxelSizeUU;
		float SizeZ = 2 * MiniVoxelSizeUU;
		FVector ImageCenter = FVector(SizeX, SizeY, SizeZ) / 2;
		DynMeshComponent->SetRelativeLocation(-ImageCenter);
		
		FFGPixelMesh PixelMesh = UFGPixelMeshBuilder::Get()->BuildMeshFromTexture(Texture2D, 2);
		
		if(PixelMesh.DynamicMesh.TriangleCount() > 0)
		{
			DynMeshComponent->SetMesh(MoveTemp(PixelMesh.DynamicMesh));
			
			DynMeshComponent->FastNotifyVertexAttributesUpdated(EMeshRenderAttributeFlags::VertexUVs);
			DynMeshComponent->FastNotifyVertexAttributesUpdated(EMeshRenderAttributeFlags::VertexNormals);
			DynMeshComponent->NotifyMeshUpdated();
		}
	}
}

#endif
