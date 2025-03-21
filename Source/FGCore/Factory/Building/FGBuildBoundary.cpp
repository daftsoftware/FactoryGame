// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGBuildBoundary.h"
#include "..\..\System\FGGameData.h"

#include "Components/BoxComponent.h"

AFGBuildBoundary::AFGBuildBoundary()
{
	RootComponent = BoundryRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));

	DesaturationBoundryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Destroy Boundry Mesh Component"));
	DesaturationBoundryMesh->SetupAttachment(BoundryRoot);
	DesaturationBoundryMesh->CastShadow = false;
	DesaturationBoundryMesh->SetRenderCustomDepth(true);
	DesaturationBoundryMesh->CustomDepthStencilValue = 5;

	// Hide the mesh in game - but do not hide it in stencil test.
	DesaturationBoundryMesh->SetRenderInMainPass(false);
	DesaturationBoundryMesh->SetRenderInDepthPass(false);

	GridBoundryMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grid Boundry Mesh Component"));
	GridBoundryMeshComponent->SetupAttachment(BoundryRoot);
	GridBoundryMeshComponent->CastShadow = false;
	
	VolumeComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume Component"));
	VolumeComponent->SetupAttachment(DesaturationBoundryMesh);
	VolumeComponent->SetBoxExtent(FVector(50.f));
}

void AFGBuildBoundary::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if(!DesaturationBoundryMesh->GetStaticMesh() || !GridBoundryMeshComponent->GetStaticMesh())
	{
		DesaturationBoundryMesh->SetStaticMesh(UFGGameData::Get().DesatMaskMesh.LoadSynchronous());
		GridBoundryMeshComponent->SetStaticMesh(UFGGameData::Get().BuildLimitMesh.LoadSynchronous());
	}

	// Snap scale to even grid sizes only, aka 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, etc.
	GridSizeSquared = (GridSizeSquared + 1) & ~1;

	// Mesh is 100x100, so scale it to match the grid size.
	DesaturationBoundryMesh->SetWorldScale3D(FVector(FVector2D(GridSizeSquared), BoundryHeight / 100.f));
	GridBoundryMeshComponent->SetWorldScale3D(FVector(FVector2D(GridSizeSquared), BoundryHeight / 100.f));

	// Offset the visualizers to sit ontop of the pivot.
	DesaturationBoundryMesh->SetRelativeLocation(FVector(FVector2D(0.f), BoundryHeight / 2.f));
	GridBoundryMeshComponent->SetRelativeLocation(FVector(FVector2D(0.f), BoundryHeight / 2.f));
	VolumeComponent->SetRelativeLocation(FVector(FVector2D(0.f), BoundryHeight / 2.f));
}
