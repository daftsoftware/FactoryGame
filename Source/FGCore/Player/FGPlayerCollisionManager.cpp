// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPlayerCollisionManager.h"

#include "FGPlayerPawn.h"
#include "World/FGVoxelSystem.h"
#include "FGVoxelDefines.h"
#include "FGVoxelUtils.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "GameFramework\SpectatorPawn.h"

AFGPlayerCollisionManager::AFGPlayerCollisionManager()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AFGPlayerCollisionManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<APawn*> PawnArray;
	
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGPlayerPawn::StaticClass(), OutActors);

	Algo::Transform(OutActors, PawnArray, [](AActor* Actor)
	{
		return Cast<APawn>(Actor);
	});
	
	int32 PoolExtensionNum = PawnArray.Num() - BoxComponentPool.Num();
	for(int32 Index = 0; Index < PoolExtensionNum; Index++)
	{
		auto* Box = NewObject<UBoxComponent>(this, *FString::Format(TEXT("PooledAABB{0}"), { BoxComponentPool.Num() }), RF_Transient);
		Box->SetBoxExtent(FVector(FG::Const::VoxelSizeUU / 2));
		Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Box->SetHiddenInGame(false);
		Box->SetVisibility(true);
		Box->SetLineThickness(5.f);
		Box->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		Box->SetUsingAbsoluteLocation(true);
		Box->SetUsingAbsoluteRotation(true);
		Box->SetUsingAbsoluteScale(true);
		Box->RegisterComponent();
		Box->CreatePhysicsState();
		AddInstanceComponent(Box);
		BoxComponentPool.Add(Box);
	}

	auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();
	
	for(int32 Pawn = 0; Pawn < PawnArray.Num(); Pawn++)
	{
		if (PawnArray[Pawn]->IsA(ASpectatorPawn::StaticClass()))
		{
			continue;
		}
		auto* RootCapsule = CastChecked<UCapsuleComponent>(PawnArray[Pawn]->GetRootComponent());
		FVector FeetLocation = RootCapsule->GetComponentLocation() - FVector::UpVector * RootCapsule->GetScaledCapsuleHalfHeight();
		FVector Floor = UFGVoxelUtils::SnapToNearestVoxel(FeetLocation - FVector::UpVector);
		
		// Has the chunk been generated locally yet?
		if(VoxSys->VoxelGrid->IsChunkGenerated(UFGVoxelUtils::VectorToChunkCoord(FeetLocation - FVector::UpVector)))
		{
			int32 VoxelType = UFGVoxelUtils::GetVoxelTypeAtLocation(GetWorld(), FeetLocation - FVector::UpVector);
		
			if(Floor.Z > FeetLocation.Z)
			{
				Floor.Z -= FG::Const::VoxelSizeUU;
			}

			if(VoxelType > VOXELTYPE_NONE) // Opaque
			{
				BoxComponentPool[Pawn]->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				//UE_LOGFMT(LogTemp, Warning, "Feet are in voxel {Voxel}", VoxelType);
			}
			else // Air
			{
				BoxComponentPool[Pawn]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			BoxComponentPool[Pawn]->SetWorldLocation(Floor + FVector(FG::Const::VoxelSizeUU / 2));
		}
		else // Hold the pawn in the air until chunk finishes loading.
		{
			BoxComponentPool[Pawn]->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			BoxComponentPool[Pawn]->SetWorldLocation(Floor + FVector(FG::Const::VoxelSizeUU / 2));
		}
	}
}