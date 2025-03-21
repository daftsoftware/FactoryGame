// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPlayerPawn.h"
#include "Player/FGPlayerController.h"
#include "Player/IFGInteractable.h"
#include "System/FGAssetManager.h"
#include "System/FGGameData.h"
#include "Inventory/FGItemStack.h"
#include "Inventory/FGItemSubsystem.h"
#include "Inventory/FGItemContainer.h"
#include "FGGameplayTags.h"
#include "FGVoxelDefines.h"
#include "FGVoxelUtils.h"

#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "World/FGVoxelSystem.h"
#include "AbilitySystem/FGAbilitySystemComponent.h"
#include "FGInputComponent.h"
#include "AbilitySystem/FGAbilitySet.h"
#include "Graphics/FGPixelMeshBuilder.h"
#include "Inventory/FGVoxelItemData.h"
#include "Net/UnrealNetwork.h"
#include "Components/DynamicMeshComponent.h"
#include "Misc/FGVoxelProjectSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGPlayerPawn)

namespace FG
{
	static bool DebugInteractionSpheres = false;
	FAutoConsoleVariableRef CVarDebugInteractionSpheres (
		TEXT("FG.DebugInteraction.Spheres"),
		DebugInteractionSpheres,
		TEXT("Draws debug spheres for interaction. (0/1)"),
		ECVF_Default
	);
	
	static bool DebugInteractionLines = false;
	FAutoConsoleVariableRef CVarDebugInteractionLines (
		TEXT("FG.DebugInteraction.Lines"),
		DebugInteractionLines,
		TEXT("Draws debug lines for interaction. (0/1)"),
		ECVF_Default
	);
	
	static float HeldItemLerpSpeed = 38.f;
	FAutoConsoleVariableRef CVarHeldItemLerpSpeed (
		TEXT("FG.HeldItemLerpSpeed"),
		HeldItemLerpSpeed,
		TEXT("Controls the lag on the held item infront of the player camera."),
		ECVF_Default
	);
}

AFGPlayerPawn::AFGPlayerPawn()
{
	using namespace FG::Const;
	
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemReplicationMode = EGameplayEffectReplicationMode::Mixed;

	HeldItemTarget = CreateDefaultSubobject<USceneComponent>(TEXT("HeldItemTarget"));
	HeldItemTarget->SetupAttachment(GetCameraComponent());

	HeldItemMesh = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("HeldItemMesh"));
	HeldItemMesh->SetUsingAbsoluteLocation(true);
	HeldItemMesh->SetUsingAbsoluteRotation(true);
	HeldItemMesh->SetUsingAbsoluteScale(true);

	TransientSlot = CreateDefaultSubobject<UFGPlayerContainer>("TransientHoldSlot");
	TransientSlot->GetItemStacks_Mutable().AddDefaulted(1);
	
	Backpack = CreateDefaultSubobject<UFGItemBackpack>("Backpack");
	Backpack->GetItemStacks_Mutable().AddDefaulted(30);

	GetCapsuleComponent()->SetCapsuleRadius(VoxelSizeUU);
	GetCapsuleComponent()->SetCapsuleHalfHeight(VoxelSizeUU * 2.0);
}

void AFGPlayerPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, QuickItemIndex);
}

void AFGPlayerPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(Backpack);
	Backpack->InitializeBackpack();
	ForceNetUpdate();
}

void AFGPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (auto* EIC = CastChecked<UFGInputComponent>(PlayerInputComponent))
	{
		if (const UFGInputConfig* InputConfig = UFGAssetManager::GetAsset(UFGGameData::Get().InputConfig))
		{
			// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
			// be triggered directly by these input actions Triggered events. 
			TArray<uint32> BindHandles;
			EIC->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, /*out*/ BindHandles);

			EIC->BindNativeAction(InputConfig, TAG_INPUT_MOVE, ETriggerEvent::Triggered, this, &ThisClass::Move, /*bLogIfNotFound=*/ false);
			EIC->BindNativeAction(InputConfig, TAG_INPUT_MOVE, ETriggerEvent::Completed, this, &ThisClass::MoveCompleted, /*bLogIfNotFound=*/ false); // why?
			EIC->BindNativeAction(InputConfig, TAG_INPUT_LOOK, ETriggerEvent::Triggered, this, &ThisClass::Look, /*bLogIfNotFound=*/ false);
			EIC->BindNativeAction(InputConfig, TAG_INPUT_JUMP, ETriggerEvent::Triggered, this, &ThisClass::Jump, /*bLogIfNotFound=*/ false);
			EIC->BindNativeAction(InputConfig, TAG_INPUT_JUMP, ETriggerEvent::Completed, this, &ThisClass::JumpCompleted, /*bLogIfNotFound=*/ false); // why?
			EIC->BindNativeAction(InputConfig, TAG_INPUT_INTERACT, ETriggerEvent::Triggered, this, &ThisClass::Interact, /*bLogIfNotFound=*/ false);
			EIC->BindNativeAction(InputConfig, TAG_INPUT_INTERACT, ETriggerEvent::Completed, this, &ThisClass::StopInteract, /*bLogIfNotFound=*/ false);
			EIC->BindNativeAction(InputConfig, TAG_INPUT_CYCLE, ETriggerEvent::Triggered, this, &ThisClass::CycleQuickItems, /*bLogIfNotFound=*/ false);
		}
	}
}

// Setup Enhanced Input Mappings.
void AFGPlayerPawn::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (AFGPlayerController* PC = Cast<AFGPlayerController>(GetController()))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings(); // PawnClientRestart can run multiple times, clear out leftover mappings.

			UInputMappingContext* IMC = UFGAssetManager::GetAsset(UFGGameData::Get().DesktopInputMappingContext);
			Subsystem->AddMappingContext(IMC, 0);
		}
	}

	checkf(GetCameraComponent(), TEXT("CameraComponent is null on outline setup!"));
}

void AFGPlayerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!GetController())
	{
		// If we still didnt get a controller, early out
		return;
	}

	HeldItemMesh->SetWorldTransform(UKismetMathLibrary::TInterpTo(
		HeldItemMesh->GetComponentTransform(),
		HeldItemTarget->GetComponentTransform(),
		DeltaSeconds,
		FG::HeldItemLerpSpeed));

	// Local chunk generation check for mover.
	//auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();
	//FIntVector LocalChunkCoordinate = UFGVoxelUtils::VectorToChunkCoord(GetActorLocation());
	//LoadedLocalChunk = VoxSys->VoxelGrid->IsChunkGenerated(LocalChunkCoordinate);

	if(APlayerCameraManager* PCM = GetController<APlayerController>()->PlayerCameraManager)
	{
		FVector XDir = FRotationMatrix(PCM->GetCameraRotation()).GetScaledAxis(EAxis::X);

		TOptional<FFGVoxelRayHit> RayHit = UFGVoxelUtils::RayVoxelIntersection(
			GetWorld(),
			PCM->GetCameraLocation(),
			PCM->GetCameraLocation() + XDir * 350.0);

		if(RayHit.IsSet())
		{
			DrawDebugBox(
				GetWorld(),
				RayHit.GetValue().VoxelCenter,
				FVector(FG::Const::VoxelSizeUU / 2.0),
				FColor::White,
				false,
				0.1,
				0,
				1.0f);
		}
	}
	
	// Interaction
	{
		const FVector XDir = FQuatRotationMatrix(GetCameraComponent()->GetComponentQuat()).GetScaledAxis(EAxis::X);
		const FVector Start = GetCameraComponent()->GetComponentLocation();
		const FVector End = Start + (XDir * UFGGameData::Get().InteractionDistance);

		UObject* NewTarget = nullptr;
		FHitResult SphereHit, LineHit, OutHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		// Sphere trace - forgiving trace with big hitbox.
		GetWorld()->SweepSingleByChannel(
			SphereHit,
			Start,
			End,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeSphere(UFGGameData::Get().InteractionTolerance),
			QueryParams
		);

		if(FG::DebugInteractionSpheres)
		{
			// @FIXME: Uses private dependency - kismet trace utils.
			//::DrawDebugSweptSphere(
			//	GetWorld(),
			//	Start,
			//	SphereHit.GetComponent() ? SphereHit.Location : End,
			//	UFGGameData::Get().InteractionTolerance,
			//	SphereHit.GetComponent() ? FColor::Red : FColor::Green,
			//	false,
			//	1.f
			//);
		}
	
		// Sphere hit successfully found interactable, run a precise trace.
		NewTarget = FindInteractable(SphereHit.GetComponent());
		if(NewTarget)
		{
			// @TODO: @FIXME: Broken - Line trace not working as expected.
			// Sometimes traces are going to origin. + Precision obviously not working.
		
			// Line trace - Catch the most precise target.
			GetWorld()->LineTraceSingleByChannel(
				LineHit,
				Start,
				End,
				ECC_Visibility,
				QueryParams
			);

			if(FG::DebugInteractionLines)
			{
				::DrawDebugLine(
					GetWorld(),
					Start,
					LineHit.GetComponent() ? OutHit.Location : End,
					LineHit.GetComponent() ? FColor::Red : FColor::Green,
					false,
					1.f
				);
			}
		}

		// Select the most precise target, falling back to sphere trace if line trace fails.
		UObject* LineTarget	= FindInteractable(LineHit.GetComponent());
		NewTarget = LineTarget ? LineTarget : NewTarget;
		OutHit = LineTarget ? LineHit : SphereHit;

		if(NewTarget) // We actually hit something interactable.
		{
			// Check for change in targets.
			if(TargetedComponent.IsValid() && TargetedComponent.Get() != NewTarget)
			{
				// Unhighlight last target.
				SetHighlight(TargetedComponent.Get(), false);
			}
		
			// Update interaction text and current target.
			InteractText = IFGInteractable::Execute_GetInteractText(NewTarget);
			TargetedComponent = OutHit.GetComponent();

			// @TODO: Interface call here to check if we should highlight comps or full actor.
			SetHighlight(NewTarget, true);
		}
		else // No valid hit.
		{
			ResetInteractionTarget();
		}
	}
}

#if UE_WITH_IRIS
void AFGPlayerPawn::BeginReplication()
{
	Super::BeginReplication();
	AddReplicatedSubObject(Backpack, ELifetimeCondition::COND_OwnerOnly);
}
#endif

void AFGPlayerPawn::SetHighlight(UObject* Target, bool HighlightEnabled, bool HighlightOwningActor)
{
	static constexpr uint8 HighlightStencilIndex = 1;

	// Try and highlight the target primitive component.
	if(UPrimitiveComponent* TargetPrim = Cast<UPrimitiveComponent>(Target))
	{
		if(HighlightOwningActor) // Highlight outer actor instead.
		{
			Target = TargetPrim->GetOwner();
		}
		else // Highlight prim directly
		{
			TargetPrim->SetRenderCustomDepth(HighlightEnabled);
			TargetPrim->SetCustomDepthStencilValue(HighlightEnabled ? HighlightStencilIndex : 0);
		}
	}

	// Try and highlight the target actor.
	if(AActor* TargetActor = Cast<AActor>(Target))
	{
		// Highlight / Unhighlight the target actors primitive components.
		TInlineComponentArray<UPrimitiveComponent*> TargetComponents(TargetActor);
		for(auto* Component : TargetComponents)
		{
			Component->SetRenderCustomDepth(HighlightEnabled);
			Component->SetCustomDepthStencilValue(HighlightEnabled ? HighlightStencilIndex : 0);
		}
	}
}

UObject* AFGPlayerPawn::FindInteractable(UObject* InObject)
{
	if(InObject && InObject->Implements<UFGInteractable>()) // Can interact with component directly.
	{
		return InObject;
	}
	else if(InObject && InObject->GetOuter()->Implements<UFGInteractable>()) // Can interact with owner actor.
	{
		return InObject->GetOuter();
	}
	return nullptr;
}

void AFGPlayerPawn::ResetInteractionTarget()
{
	SetHighlight(TargetedComponent.Get(), false);
	
	// Reset interaction targets.
	TargetedComponent.Reset();
	InteractText = FText::FromString("");
}

void AFGPlayerPawn::Interact()
{
	if(TargetedComponent.IsValid() && TargetedComponent->GetOwner())
	{
		// Select target components owner, or the component itself if it has no owner.
		UObject* Target = FindInteractable(TargetedComponent.Get());

		if(!Target)
		{
			return;
		}
			
		if(IFGInteractable::Execute_CanInteractWith(Target, this))
		{
			CurrentlyInteracting = Target;
			IFGInteractable::Execute_StartUse(Target, this);
		}
	}
}

void AFGPlayerPawn::StopInteract()
{
	if(CurrentlyInteracting.IsValid())
	{
		IFGInteractable::Execute_StopUse(CurrentlyInteracting.Get(), this);
		CurrentlyInteracting.Reset();
	}
}

void AFGPlayerPawn::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputTagPressed(InputTag);
	}
}

void AFGPlayerPawn::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputTagReleased(InputTag);
	}
}

bool AFGPlayerPawn::IsPredictingClient() const
{
	return !HasAuthority() && IsLocallyControlled();
}

void AFGPlayerPawn::ServerPlaceStack_Implementation(int32 ToSlot, UFGItemContainer* ToContainer, int32 Amount)
{
	if (!ToContainer)
	{
		// TODO: Drop???
		return;
	}

	if (!TransientSlot->IsValidSlot(0))
	{
		// If for whatever reason we try to place an invalid slot, we early out.
		return;
	}

	if (Amount == -1)
	{
		TransientSlot->MoveStack(0, ToSlot, ToContainer);
	}
	else
	{
		TransientSlot->SplitStack(0, ToSlot, Amount, ToContainer);
	}

	ClientSetHeldStack(TransientSlot->GetItemStacks()[0]);
}

void AFGPlayerPawn::LocalHoldStack(UFGItemContainer* FromContainer, int32 FromSlot, bool bSplit)
{
	if (!FromContainer || !IsPredictingClient())
	{
		return;
	}

	if (!FromContainer->IsValidSlot(FromSlot))
	{
		return;
	}

	FFGItemStack ItemStack(FromContainer->GetItemStacks()[FromSlot]);
	if (bSplit)
	{
		ItemStack.StackSize /= 2;
	}

	TransientSlot->TransferToSlot(0, ItemStack);
}

void AFGPlayerPawn::ClientSetHeldStack_Implementation(const FFGItemStack& Item)
{
	// We can use mutable here as this is a correction
	TransientSlot->GetItemStacks_Mutable()[0] = Item;
}

void AFGPlayerPawn::ServerTryHoldStack_Implementation(UFGItemContainer* FromContainer, int32 FromSlot, bool bSplit)
{
	if (!FromContainer || !FromContainer->IsValidSlot(FromSlot))
	{
		return;
	}

	if (bSplit)
	{
		const int32 HalfStackSize = FromContainer->GetItemStacks()[FromSlot].StackSize / 2;
		FromContainer->SplitStack(FromSlot, 0, HalfStackSize, TransientSlot);
	}
	else
	{
		FromContainer->MoveStack(FromSlot, 0, TransientSlot);
	}

	// We correct the client with whatever we grabbed.
	ClientSetHeldStack(TransientSlot->GetItemStacks()[0]);
}

void AFGPlayerPawn::CycleQuickItems(const FInputActionValue& Value)
{
	if(ActiveModes.IsEmpty())
	{
		const int32 CycleDir = FMath::Sign(Value.Get<float>());
        QuickSwapToItem(((QuickItemIndex + CycleDir) + UFGGameData::Get().QuickBarSize) % UFGGameData::Get().QuickBarSize);
	}
}

void AFGPlayerPawn::QuickSwapToItem_Implementation(int32 Index)
{
	if (Index == QuickItemIndex)
	{
		return;
	}

	GetBackpack().UnEquip(QuickItemIndex);
	
	QuickItemIndex = Index;
	
	if ((*Backpack)[Index]->StaticData)
	{
		GetBackpack().Equip(Index);
	}

	OnRep_QuickItemIndex();
}

/*
	Throw chucks the item in the direction player is looking,
	otherwise just drop item at feet of the player.
*/
void AFGPlayerPawn::DropCurrentItem(bool bInitiateThrow)
{
	// @TODO: stub
}

void AFGPlayerPawn::OnRep_QuickItemIndex()
{
	OnChangeQuickSlot.Broadcast(QuickItemIndex);
	SetHeldItemMesh(QuickItemIndex);
}

void AFGPlayerPawn::SetHeldItemMesh(int32 ItemStackIndex)
{
	const FFGItemStack* ItemStack = GetItemStack(ItemStackIndex);

	// Clear Mesh.
	HeldItemMesh->SetMesh(FDynamicMesh3());

	const UFGVoxelProjectSettings* VoxelSettings = GetDefault<UFGVoxelProjectSettings>();
	if(UMaterialInterface* PixelMeshShader = VoxelSettings->PixelMeshShader.LoadSynchronous())
	{
		HeldItemMesh->SetMaterial(0, PixelMeshShader);
	}
	
	if(const auto& VoxelItemData = Cast<UFGVoxelItemData>(ItemStack->StaticData)) // Voxel Block Mesh.
	{
		UFGVoxelMetadata* Meta = VoxelItemData->VoxelMetadata;
	}
	else if(const auto& ItemData = ItemStack->StaticData) // Icon based mesh.
	{
		if(ItemData->HeldTexture.IsSet())
		{
			UTexture2D* HeldTexture = ItemData->HeldTexture.GetValue();
			
			FFGPixelMesh PixelMesh = UFGPixelMeshBuilder::Get()->BuildMeshFromTexture(
				HeldTexture, ItemData->HeldThickness, ItemData->HeldVoxelSize);

			HeldItemMesh->SetMesh(MoveTemp(PixelMesh.DynamicMesh));
		}
	}
}