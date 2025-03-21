// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGItemBuildTool.h"

#if 0 // Legacy build system - needs porting to GAS

#include "Factory/Building/FGBuildGridVisualizer.h"
#include "Factory/Building/FGBuildRotationVisualizer.h"
#include "Factory/Building/FGBuildSubsystem.h"
#include "Player/FGPlayerPawn.h"
#include "System/FGGameData.h"
#include "FGGameplayTags.h"

#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "MaterialDomain.h"
#include "Logging/StructuredLog.h"

namespace FG
{
	static float BuildToolGhostLerpSpeed = 38.f;
	FAutoConsoleVariableRef CVarBuildToolGhostLerpSpeed (
		TEXT("FG.BuildToolGhostLerpSpeed"),
		BuildToolGhostLerpSpeed,
		TEXT("Controls the lag speed on the build tool ghost."),
		ECVF_Default
	);

	static bool BuildToolDebugDraw = true;
	FAutoConsoleVariableRef CVarBuildToolDebugDraw (
		TEXT("FG.BuildToolDebugDraw"),
		BuildToolDebugDraw,
		TEXT("Enable debug draw for the build tool."),
		ECVF_Default
	);
	
	static float MinBuildDistance = 250.f;
	FAutoConsoleVariableRef CVarMinBuildDistance (
    	TEXT("FG.MinBuildDistance"),
    	MinBuildDistance,
    	TEXT("Controls the minimum build distance scrolling."),
    	ECVF_Default
    );
	
	static float MaxBuildDistance = 1000.f;
    FAutoConsoleVariableRef CVarMaxBuildDistance (
    	TEXT("FG.MaxBuildDistance"),
    	MaxBuildDistance,
    	TEXT("Controls the maximum build distance scrolling."),
    	ECVF_Default
    );
	
	static float DefaultBuildDistance = 500.f;
	FAutoConsoleVariableRef CVarDefaultBuildDistance (
    	TEXT("FG.DefaultBuildDistance"),
    	DefaultBuildDistance,
    	TEXT("Controls the default build distance before scrolling."),
    	ECVF_Default
    );
	
	static float BuildScrollSpeed = 100.f;
	FAutoConsoleVariableRef CVarBuildScrollSpeed (
    	TEXT("FG.BuildScrollSpeed"),
    	BuildScrollSpeed,
    	TEXT("Controls the scrolling speed of building."),
    	ECVF_Default
    );
}

FVector QuantizeNormal26DOF(FVector Normal)
{
	// Reclamp incase of bad input.
	Normal = Normal.GetSafeNormal();

	// 26 DOF table with normalization baked in.
	static TArray<FVector> DOFTable26 = {
		// FACES
		FVector(0.f, 0.f, 1.f),					// 0 - Up
		FVector(0.f, 0.f, -1.f),				// 1 - Down
		FVector(0.f, 1.f, 0.f),					// 2 - Right
		FVector(0.f, -1.f, 0.f),				// 3 - Left
		FVector(1.f, 0.f, 0.f),					// 4 - Forward
		FVector(-1.f, 0.f, 0.f),				// 5 - Back
		// EDGES
		FVector(0.7071f, 0.7071f, 0.0f), 		// 6 - Up Right
		FVector(-0.7071f, -0.7071f, 0.0f), 		// 7 - Down Left
		FVector(0.7071f, -0.7071f, 0.0f), 		// 8 - Up Left
		FVector(-0.7071f, 0.7071f, 0.0f), 		// 9 - Down Right
		FVector(0.7071f, 0.0f, 0.7071f), 		// 10 - Up Forward
		FVector(-0.7071f, 0.0f, -0.7071f), 		// 11 - Down Back
		FVector(0.7071f, 0.0f, -0.7071f), 		// 12 - Up Back
		FVector(-0.7071f, 0.0f, 0.7071f), 		// 13 - Down Forward
		FVector(0.0f, 0.7071f, 0.7071f), 		// 14 - Right Forward
		FVector(0.0f, -0.7071f, -0.7071f), 		// 15 - Left Back
		FVector(0.0f, 0.7071f, -0.7071f), 		// 16 - Right Back
		FVector(0.0f, -0.7071f, 0.7071f), 		// 17 - Left Forward
		// CORNERS
		FVector(0.5774f, 0.5774f, 0.5774f), 	// 18 - Up Right Forward
		FVector(-0.5774f, -0.5774f, -0.5774f),	// 19 - Down Left Back
		FVector(0.5774f, 0.5774f, -0.5774f), 	// 20 - Up Right Back
		FVector(-0.5774f, -0.5774f, 0.5774f), 	// 21 - Down Left Forward
		FVector(0.5774f, -0.5774f, 0.5774f), 	// 22 - Up Left Forward
		FVector(-0.5774f, 0.5774f, -0.5774f), 	// 23 - Down Right Back
		FVector(0.5774f, -0.5774f, -0.5774f), 	// 24 - Up Left Back
		FVector(-0.5774f, 0.5774f, 0.5774f) 	// 25 - Down Right Forward
	};

	// Pick the closest DOF from the table.
	double		ClosestDistance	= -1.f;
	FVector*	Closest			= nullptr;
	for(FVector& DOF : DOFTable26)
	{
		if((DOF | Normal) > ClosestDistance)
		{
			Closest = &DOF;
			ClosestDistance = (DOF | Normal);
		}
	}
	return *Closest;
}

void UFGBuildToolStartPlacingAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	GridVisualizer = GetWorld()->SpawnActor<AFGBuildGridVisualizer>(
		AFGBuildGridVisualizer::StaticClass()
	);
	
	RotationVisualizer = GetWorld()->SpawnActor<AFGBuildRotationVisualizer>(
		AFGBuildRotationVisualizer::StaticClass()
	);

	UE_LOGFMT(LogTemp, Display, "Start Primary Action");
	
	// Enter Place Mode or Place
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	auto* Pawn = PC->GetPawn<AFGPlayerPawn>();

	if(!PlacingActor) // Begin Placing
	{
		StartPlacing();
	}
	else // End Placing
	{
		Place();
		StartPlacing();
	}
}

void UFGBuildToolStartPlacingAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOGFMT(LogTemp, Display, "Stop Primary Action");

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	auto* Pawn = PC->GetPawn<AFGPlayerPawn>();
	
	// Reset fast build setting.
	Pawn->FastBuildIndex = 0;

	// @TODO: Poll CDO for these:
	bFastBuild = false;
	FastBuildCounter = 0.0f;
	FastBuildRate = 0.5f;
	FastBuildInitialXForm = FTransform::Identity;
	FastBuildDir = FVector::ZeroVector;
}

void UFGBuildToolStartPlacingAbility::Tick(float DeltaTime)
{
	if(PlacingActor)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		auto* Pawn = PC->GetPawn<AFGPlayerPawn>();

		if(Pawn)
		{
			FTransform Offset = FTransform( // Forward Vector * Build Distance
				FRotator::ZeroRotator,
				(FVector::ForwardVector * Pawn->BuildDistance),
				FVector::OneVector
			);

			// @FIXME: Problematic on fast building. - We need to take the last build xform instead.
			FTransform PlacingActorXForm = Offset * Pawn->GetCameraComponent()->GetComponentTransform();

			PlacingActorXForm.SetRotation(Pawn->BuildRotation.Quaternion()); // Align to current build rotation.

			PlacingActorXForm.SetLocation( // Align location to Grid.
				PlacingActorXForm.GetLocation().GridSnap(UFGGameData::Get().GridSize)
			);

			//	Apply fast build offset.
			if(!Pawn->BuildHistory.IsEmpty()) // Must have placed atleast a block before. @TODO: Suggestive fastbuild from snap.
			{
				if(Pawn->FastBuildIndex > 0) // Continuation blocks of the fast build.
				{
					// Scale the fast build dir (normalized) to the next grid in the direction.
					// We can't just flat multiply by the grid size because the travel for corners and edges
					// is different to faces.
					static TArray<float> GridOffsets = {
						0.f,		// Invalid
						100.f,		// Face
						141.42f,	// Edge
						173.21f,	// Corner
					};

					// Check the number of non zero'd components on the output normal.
					// from this we can determine face, edge or corner.
					// A face will have 2 non zero comps, an edge will have 1 and a corner will have 0.
					int32 NonZeroCount = 0;
					if(FastBuildDir.X != 0.f) NonZeroCount++;
					if(FastBuildDir.Y != 0.f) NonZeroCount++;
					if(FastBuildDir.Z != 0.f) NonZeroCount++;

					float ChosenGridOffset = GridOffsets[NonZeroCount];
					UE_LOGFMT(LogTemp, Display, "FAST BUILD GRID OFFSET - {}", ChosenGridOffset);

					if(FG::PlacerDebug && !Pawn->BuildHistory.IsEmpty())
					{
						DrawDebugDirectionalArrow(
							GetWorld(),
							Pawn->BuildHistory[0].Get()->GetActorLocation(),
							Pawn->BuildHistory[0].Get()->GetActorLocation() + (FastBuildDir * ChosenGridOffset),
							10.f,
							FColor::Red,
							false,
							0.1f,
							0,
							10.f);
					}
								
					FTransform FastBuildOffset = FTransform(
						FRotator::ZeroRotator,
						(FastBuildDir * ChosenGridOffset) * (Pawn->FastBuildIndex + 1),
						FVector::OneVector
					);
					PlacingActorXForm = FastBuildOffset * FastBuildInitialXForm; // Apply fast build offset.
				}
				else // First block placed in fast build.
				{
					// Cache the last built block xform for Fast Build.
					FastBuildInitialXForm = Pawn->BuildHistory[0].Get()->GetActorTransform();

					// Calculate fast build direction.
					FastBuildDir = QuantizeNormal26DOF(
						PlacingActorXForm.GetLocation() - FastBuildInitialXForm.GetLocation()
					);
				}
			}

			// Move placeable.
			PlacingActor->SetActorTransform(UKismetMathLibrary::TInterpTo(
				PlacingActor->GetActorTransform(),
				PlacingActorXForm,
				DeltaSeconds,
				FG::PlaceActorLerpSpeed
			));

			// Move visualizers.
			GridVisualizer->SetActorTransform(PlacingActorXForm);
			RotationVisualizer->SetActorTransform(PlacingActorXForm);
			RotationVisualizer->SetActorRotation(FQuat::Identity);

			// Show visualizer for rotation.
			RotationVisualizer->UpdateAxis(Pawn->ActiveModes);

			auto* BuildSys = GetWorld()->GetSubsystem<UFGBuildSubsystem>();
			
			// Check if something occupies the hovered grid.
			if(BuildSys->BuildGrid.Contains(UFGBuildSubsystem::QuantizeToGrid(PlacingActor->GetActorLocation())))
			{
				GridVisualizer->SetColor(FLinearColor::Red);
			}
			else
			{
				GridVisualizer->SetColor(FLinearColor::Green);
			}
		}

		if(bFastBuild)
		{
			FastBuildCounter += DeltaSeconds;
            if(FastBuildCounter > FastBuildRate) // Check if we can fast build.
            {
            	FastBuildCounter -= FastBuildRate;

            	UE_LOG(LogTemp, Display, TEXT("fast placed"));

            	// Speed up fast build.
				FastBuildRate = FMath::Max(FastBuildMaxRate, FastBuildRate - FastBuildAcceleration);
            	
            	if(!Place()) // Try place, if it fails, stop fast building.
            	{
            		bFastBuild = false;
            	}
            	
            	StartPlacing();
            }
		}
	}
}

void UFGBuildToolStartPlacingAbility::ToggleGhost(AActor* Actor, bool NewValue)
{
	TInlineComponentArray<UStaticMeshComponent*> MeshComponents;
	Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

	if(MeshComponents.IsEmpty())
	{
		return;
	}
	
	if(NewValue)
	{
		auto* GhostMaterial = UFGGameData::Get().BuildableGhostMaterial.LoadSynchronous();
		checkf(GhostMaterial, TEXT("Invalid ghost material."));

		// Iterate mesh components, setting materials to ghost visuals.
		for(UStaticMeshComponent* MeshComponent : MeshComponents)
		{
			for(int32 Mat = 0; Mat < MeshComponent->GetNumMaterials(); Mat++)
			{
				MeshComponent->SetMaterial(Mat, GhostMaterial);
			}
		}
	}
	else
	{
		TArray<UStaticMeshComponent*> CDOMeshComponents;

		// Can't use TInlineComponentArray + GetComponents here because it doesn't find blueprint
		// generated components. Instead we have to manually iterate with this bollocks. GG Epic.
		{
			UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(Actor->GetClass());
			const TArray<USCS_Node*>& ActorBlueprintNodes = BPClass->SimpleConstructionScript->GetAllNodes();
			
			for(auto* Node : ActorBlueprintNodes)
			{
				if(Node->ComponentClass == UStaticMeshComponent::StaticClass())
				{
					CDOMeshComponents.Add(Cast<UStaticMeshComponent>(Node->ComponentTemplate));
				}
			}
		}

		// Iterate mesh components, resetting materials to their default visuals.
		for(int32 MeshComp = 0; MeshComp < CDOMeshComponents.Num(); MeshComp++)
		{
			for(int32 Mat = 0; Mat < CDOMeshComponents[MeshComp]->GetNumMaterials(); Mat++)
			{
				UMaterialInterface* CDOMat = CDOMeshComponents[MeshComp]->GetMaterial(Mat);
				MeshComponents[MeshComp]->SetMaterial(
					Mat,
					CDOMat ? CDOMat : UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface)
				);
			}
		}
	}
}

void UFGBuildToolStartPlacingAbility::StartPlacing()
{
	// Play start build SFX.
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	auto* Pawn = PC->GetPawn<AFGPlayerPawn>();

	checkf(!UFGGameData::Get().BuildStartSFX.IsNull(), TEXT("Invalid build start SFX."));
	UGameplayStatics::PlaySound2D(
		GetWorld(),
		UFGGameData::Get().BuildStartSFX.LoadSynchronous() // @FIXME: Load sync.
	);
	
	// Set pawn in building mode.
	Pawn->ActiveModes.AddTag(TAG_PLAYER_MODES_BUILDING);
	
	UClass* PlacementClass = GetStaticData<UFGBuildableStaticData>()->PlacedClass.LoadSynchronous();
	checkf(PlacementClass, TEXT("Invalid placement class in buildable static data."));
	
	// Create instance of the buildable to place.
	PlacingActor = GetWorld()->SpawnActor<AActor>(
		PlacementClass,
		FTransform(
			FRotator::ZeroRotator,
			Pawn->HeldItemMesh->GetComponentLocation(),
			FVector::ZeroVector
		)
	);

	// Show grid.
	GridVisualizer->SetActorHiddenInGame(false);
	
	ToggleGhost(PlacingActor, true);

	PlacingActor->SetActorEnableCollision(false);
}

bool UFGBuildToolStartPlacingAbility::Place()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	auto* Pawn = PC->GetPawn<AFGPlayerPawn>();

	auto* BuildSys = GetWorld()->GetSubsystem<UFGBuildSubsystem>();

	// Check something doesn't already occupy that grid.
	if(BuildSys->BuildGrid.Contains(UFGBuildSubsystem::QuantizeToGrid(PlacingActor->GetActorLocation())))
	{
		// Play error SFX.
		checkf(!UFGGameData::Get().BuildBlockedSFX.IsNull(), TEXT("Invalid build blocked SFX."));
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			UFGGameData::Get().BuildBlockedSFX.LoadSynchronous(), // @FIXME: Load sync.
			PlacingActor->GetActorLocation()
		);

		// @TODO: Flash red.
		
		return false;
	}

	BuildSys->BuildGrid.Add(
		UFGBuildSubsystem::QuantizeToGrid(PlacingActor->GetActorLocation()),
		PlacingActor
	);
	
	// Add to build history.
	Pawn->BuildHistory.Insert(PlacingActor, 0);
	if(Pawn->BuildHistory.Num() > Pawn->BuildHistoryLength)
	{
		Pawn->BuildHistory.RemoveAt(Pawn->BuildHistoryLength);
	}

	Pawn->FastBuildIndex++;

	// Snap to grid.
	PlacingActor->SetActorLocation(
		PlacingActor->GetActorLocation().GridSnap(UFGGameData::Get().GridSize)
	);
	
	// Restore collision setting.
	 PlacingActor->SetActorEnableCollision(
		PlacingActor->GetClass()->GetDefaultObject<AActor>()->GetActorEnableCollision()
	 );

	// Play SFX.
	checkf(!UFGGameData::Get().BuildPlacementSFX.IsNull(), TEXT("Invalid build place SFX."));
	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		UFGGameData::Get().BuildPlacementSFX.LoadSynchronous(), // @FIXME: Load sync.
		PlacingActor->GetActorLocation()
	);
	
	ToggleGhost(PlacingActor, false);
	PlacingActor = nullptr;
	return true;
}

void UFGBuildToolCancelPlacingAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
    auto* Pawn = PC->GetPawn<AFGPlayerPawn>();
    
    if(PlacingActor) // Cancel any current placement.
    {
    	PlacingActor->Destroy();
    	PlacingActor = nullptr;
    }
    
    // Stop pawn being in building mode.
    Pawn->ActiveModes.RemoveTag(TAG_PLAYER_MODES_BUILDING);
    
    // Hide grid.
    GridVisualizer->SetActorHiddenInGame(true);
    
    // @TODO: Who asked moment?
    Pawn->ResetBuildDistance();
}

void UFGBuildToolChangeDistanceAbility::ScrollBuildDistance(const FInputActionValue& Value)
{
	static TArray<FGameplayTag> RotatorModes = { // @TODO: Essentially just a shit bitmask
		TAG_PLAYER_MODES_ROTATEX,
		TAG_PLAYER_MODES_ROTATEY,
		TAG_PLAYER_MODES_ROTATEZ
	};
	
	if(ActiveModes.HasAny(FGameplayTagContainer::CreateFromArray(RotatorModes))) // Rotate buildable with scroll
	{
		// Apply build rotation.
		BuildRotation += FRotator::MakeFromEuler(LastRotateModeInput) * (90.f * Value.Get<float>());
		UE_LOG(LogTemp, Display, TEXT("Updated Build Rotation - %s"), *BuildRotation.ToString());
	}
	else if(ActiveModes.HasTag(TAG_PLAYER_MODES_BUILDING)) // Scroll buildable distance.
	{
		BuildDistance = FMath::Clamp(
			BuildDistance + (Value.Get<float>() * FG::BuildScrollSpeed),
			FG::MinBuildDistance,
			FG::MaxBuildDistance
		);

		UE_LOG(LogTemp, Display, TEXT("Build Distance: %f"), BuildDistance);
	}
}

void UFGBuildToolChangeDistanceAbility::ResetBuildDistance()
{
 	BuildDistance = FG::DefaultBuildDistance;
}

void UFGBuildToolChangeDistanceAbility::BuildRotateMode(const FInputActionValue& Value)
{
	FVector BuildRotateModeInput = Value.Get<FVector>();
	LastRotateModeInput = BuildRotateModeInput;

	// @TODO: This will 100% be cleaner as a bitmask - refactor?
	// Kind of nice and simple having all the modes in one place, dunno.

	if(BuildRotateModeInput.X > 0.f) // Rotate X
	{
		ActiveModes.AddTag(TAG_PLAYER_MODES_ROTATEX);
	}
	else
	{
		ActiveModes.RemoveTag(TAG_PLAYER_MODES_ROTATEX);
	}

	if(BuildRotateModeInput.Y > 0.f) // Rotate Y
	{
		ActiveModes.AddTag(TAG_PLAYER_MODES_ROTATEY);
	}
	else
	{
		ActiveModes.RemoveTag(TAG_PLAYER_MODES_ROTATEY);
	}

	if(BuildRotateModeInput.Z > 0.f) // Rotate Z
	{
		ActiveModes.AddTag(TAG_PLAYER_MODES_ROTATEZ);
	}
	else
	{
		ActiveModes.RemoveTag(TAG_PLAYER_MODES_ROTATEZ);
	}
}

#endif