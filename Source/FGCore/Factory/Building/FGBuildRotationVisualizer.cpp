// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGBuildRotationVisualizer.h"
#include "..\..\System\FGGameData.h"
#include "..\..\FGGameplayTags.h"

#include "GameplayTagContainer.h"

/*
	Outrageously terrible code, but idc works for now.
	Got halfway through realized it sucked, but can't
	go back on it now xd
*/

AFGBuildRotationVisualizer::AFGBuildRotationVisualizer()
{
	RootComponent = RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
}

void AFGBuildRotationVisualizer::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	UStaticMesh* RingMesh = UFGGameData::Get().RotatorRingMesh.LoadSynchronous();
	checkf(RingMesh, TEXT("Ring mesh is null."));
	
	for(int32 Axis = 1; Axis < 4; Axis++) // Start at 1 to skip EAxis::None.
	{
		TEnumAsByte<EAxis::Type> AxisEnum(Axis);

		RingMeshes.Add(AxisEnum,
			NewObject<UStaticMeshComponent>(
				this,
				*FString::Printf(TEXT("RingMesh_%d"), Axis)
			)
		);
		
		RingMeshes[AxisEnum]->RegisterComponent();
		RingMeshes[AxisEnum]->AttachToComponent(RootScene, FAttachmentTransformRules::KeepRelativeTransform);
		RingMeshes[AxisEnum]->SetStaticMesh(RingMesh);
		RingMeshes[AxisEnum]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RingMeshes[AxisEnum]->SetHiddenInGame(true);

		// Pick a color based on X Y Z
		FVector Color = FVector::ZeroVector;
        Color[Axis - 1] = 1.f;
        RingMeshes[AxisEnum]->SetCustomPrimitiveDataVector3(0, Color);
	}
}

void AFGBuildRotationVisualizer::UpdateAxis(FGameplayTagContainer PlayerModes)
{
	if(PlayerModes.HasTag(TAG_PLAYER_MODES_ROTATEX))
	{
		RingMeshes[EAxis::X]->SetRelativeRotation(FVector::ForwardVector.Rotation());
		RingMeshes[EAxis::X]->SetHiddenInGame(false);
	}
	else
	{
		RingMeshes[EAxis::X]->SetHiddenInGame(true);
	}

	if(PlayerModes.HasTag(TAG_PLAYER_MODES_ROTATEY))
	{
		RingMeshes[EAxis::Y]->SetRelativeRotation(FVector::RightVector.Rotation());
		RingMeshes[EAxis::Y]->SetHiddenInGame(false);
	}
	else
    {
    	RingMeshes[EAxis::Y]->SetHiddenInGame(true);
    }
	
	if(PlayerModes.HasTag(TAG_PLAYER_MODES_ROTATEZ))
	{
		RingMeshes[EAxis::Z]->SetRelativeRotation(FVector::UpVector.Rotation());
		RingMeshes[EAxis::Z]->SetHiddenInGame(false);
	}
	else
    {
		RingMeshes[EAxis::Z]->SetHiddenInGame(true);
    }
}

