// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGBuildGridVisualizer.h"

#include "..\..\System\FGGameData.h"

namespace FG
{
	static int32 GridVisualizerSize = 5;
	FAutoConsoleVariableRef CVarGridVisualizerSize (
		TEXT("FG.GridVisualizerSize"),
		GridVisualizerSize,
		TEXT("How many grids to visualize when placing buildables."),
		ECVF_Default
	);
}

AFGBuildGridVisualizer::AFGBuildGridVisualizer()
{
	RootComponent = RootScene = CreateDefaultSubobject<USceneComponent>("RootScene");
}

void AFGBuildGridVisualizer::BeginPlay()
{
	Super::BeginPlay();

	RebuildGrid();
	SetColor(FLinearColor::Green);
}

void AFGBuildGridVisualizer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RebuildGrid();
	SetColor(FLinearColor::Green);
}

void AFGBuildGridVisualizer::SetColor(FLinearColor Color)
{
	for(UStaticMeshComponent* GridPlane : GridPlanes)
	{
		GridPlane->SetDefaultCustomPrimitiveDataVector4(0, FVector4(Color));
	}
}

void AFGBuildGridVisualizer::RebuildGrid()
{
	GridPlaneMesh = UFGGameData::Get().GridPlaneMesh.LoadSynchronous(); // @FIXME: LoadSync

	if(!GridPlaneMesh || !GridPlanes.IsEmpty()) // Early Out
	{
		return;
	}
	
	// Iterate degrees of freedom.
	for(int32 DOF = 0; DOF < 6; DOF++)
	{
		// Iterate planes to create on a given DOF.
		for(int32 Plane = 0; Plane < FG::GridVisualizerSize / 2; Plane++)
		{
			UStaticMeshComponent* GridPlane = NewObject<UStaticMeshComponent>(this);
			GridPlane->RegisterComponent();
			GridPlane->SetStaticMesh(GridPlaneMesh);
			GridPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GridPlane->SetCollisionProfileName(TEXT("NoCollision"));
			GridPlane->SetMobility(EComponentMobility::Movable);
			GridPlane->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			GridPlanes.Add(GridPlane);

			GridPlane->SetRelativeScale3D(
				FVector(FG::GridVisualizerSize) * (UFGGameData::Get().GridSize / 100.f)
			);

			switch(DOF) // Switch Rotation on DOF
			{
			case 0: // +X
				GridPlane->SetRelativeLocation(FVector(50.f, 0.f, 0.f) + (FVector::ForwardVector * (UFGGameData::Get().GridSize * Plane)));
				GridPlane->SetWorldRotation(FVector::ForwardVector.ToOrientationQuat());
				break;
			case 1: // -X
				GridPlane->SetRelativeLocation(FVector(-50.f, 0.f, 0.f) + (-FVector::ForwardVector * (UFGGameData::Get().GridSize * Plane)));
				GridPlane->SetWorldRotation(-FVector::ForwardVector.ToOrientationQuat());
				break;
			case 2: // +Y
				GridPlane->SetRelativeLocation(FVector(0.f, 50.f, 0.f) + (FVector::RightVector * (UFGGameData::Get().GridSize * Plane)));
				GridPlane->SetWorldRotation(FVector::RightVector.ToOrientationQuat());
				break;
			case 3: // -Y
				GridPlane->SetRelativeLocation(FVector(00.f, -50.f, 0.f) + (-FVector::RightVector * (UFGGameData::Get().GridSize * Plane)));
				GridPlane->SetWorldRotation(-FVector::RightVector.ToOrientationQuat());
				break;
			case 4: // +Z
				GridPlane->SetRelativeLocation(FVector(0.f, 0.f, 50.f) + (FVector::UpVector * (UFGGameData::Get().GridSize * Plane)));
				GridPlane->SetWorldRotation(FVector::UpVector.ToOrientationQuat());
				break;
			case 5: // -Z
				GridPlane->SetRelativeLocation(FVector(0.f, 0.f, -50.f) + (-FVector::UpVector * (UFGGameData::Get().GridSize * Plane)));
				GridPlane->SetWorldRotation(-FVector::UpVector.ToOrientationQuat());
				break;
			}
		}
	}
}
