// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelPlaceAbility.h"
#include "System/FGAssetManager.h"
#include "System/FGGameData.h"
#include "World/FGVoxelSystem.h"
#include "FGVoxelDefines.h"
#include "FGVoxelUtils.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Engine/StreamableManager.h"
#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "ExtendableGameStateBase.h"
#include "AbilitySystem/FGGameplayAbilityTargetDataVoxelType.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Player/FGPlayerPawn.h"
#include "Inventory/FGVoxelItemData.h"
#include "Inventory/FGItemStack.h" 
#include "Inventory/FGItemContainer.h" 
#include "Online/FGVoxelNetManager.h"

namespace FG
{
	static int32 VoxelPlaceId = 1;
	FAutoConsoleVariableRef CVarVoxelPlaceId (
		TEXT("FG.VoxelPlaceId"),
		VoxelPlaceId,
		TEXT("Id of the voxel place ability."),
		ECVF_Default
	);
}

UFGVoxelPlaceAbility::UFGVoxelPlaceAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UFGVoxelPlaceAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
	ReleaseTask->ReadyForActivation();

	UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, 0.25f);
	WaitTask->OnFinish.AddDynamic(this, &ThisClass::K2_EndAbilityLocally);
	WaitTask->ReadyForActivation();

	// Bind target data callback
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	if (ActorInfo->IsLocallyControlled())
	{
		if (APlayerCameraManager* PCM = ActorInfo->PlayerController->PlayerCameraManager)
		{
			FVector XDir = FRotationMatrix(PCM->GetCameraRotation()).GetScaledAxis(EAxis::X);

			TOptional<FFGVoxelRayHit> RayHit = UFGVoxelUtils::RayVoxelIntersection(
				GetWorld(),
				PCM->GetCameraLocation(),
				PCM->GetCameraLocation() + XDir * 350.0);

			if (!RayHit.IsSet())
			{
				Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
				return;
			}

			FVector End = RayHit.GetValue().VoxelCenter + (RayHit->GetHitFaceNormal() * FG::Const::VoxelSizeUU);

			// Fill out the target data from the hit results
			FGameplayAbilityTargetData_VoxelInfo* VoxelTypeTargetData = new FGameplayAbilityTargetData_VoxelInfo();
			VoxelTypeTargetData->TargetLocation.LiteralTransform = FTransform(End);

			UFGEquipmentSlot* Slot = GetEquipmentSlot();
			const FFGItemStack* ItemStack = CastChecked<AFGPlayerPawn>(GetAvatarActorFromActorInfo())->GetItemStack(Slot->BackpackIndex);

			VoxelTypeTargetData->VoxelType = CastChecked<UFGVoxelItemData>(ItemStack->StaticData)->VoxelMetadata->GetVoxelType();
			
			FGameplayAbilityTargetDataHandle TargetData;
			TargetData.Add(VoxelTypeTargetData);

			// Process the target data immediately
			OnTargetDataReadyCallback(TargetData, FGameplayTag());
		}
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UFGVoxelPlaceAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);

		// When ability ends, consume target data and remove delegate
		MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UFGVoxelPlaceAbility::PlaceVoxel(const FVector& Position, const int32 VoxelType)
{
	auto* VoxSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();

	DrawDebugBox(
		GetWorld(),
		Position,
		FVector(FG::Const::VoxelSizeUU / 2.0),
		FColor::Red,
		false,
		1.0);

	FIntVector ChunkCoordinate = UFGVoxelUtils::VectorToChunkCoord(Position);
	FIntVector VoxelCoordinate = UFGVoxelUtils::VectorToVoxelCoord(Position);

	if (VoxSys->VoxelGrid->IsChunkGenerated(ChunkCoordinate))
	{
		if (GetActorInfo().IsNetAuthority()) // Multicast.
		{
			for(auto* PC : TActorRange<APlayerController>(GetWorld()))
			{
				auto* NetMgr = PC->FindComponentByClass<UFGVoxelNetManager>();
				NetMgr->ClientReceiveAddressedVoxelUpdate(FFGVoxelNetRef(ChunkCoordinate, VoxelCoordinate), VoxelType);
			}
		}
		else // Local Only Prediction.
		{
			VoxSys->ModifyVoxel(ChunkCoordinate, VoxelCoordinate, VoxelType);
		}

		TSoftObjectPtr<> PlaceSound = UFGGameData::Get().PlaceSFX;
		FStreamableManager& StreamableMgr = UFGAssetManager::GetStreamableManager();

		// Play place SFX.
		StreamableMgr.RequestAsyncLoad(PlaceSound.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, Position, PlaceSound]()
			{
				check(IsInGameThread());

				auto* LoadedSound = CastChecked<USoundBase>(PlaceSound.Get());
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), LoadedSound, Position);
			}));
	}
	else
	{
		UE_LOGFMT(LogTemp, Error, "Tried to modify voxel when chunk was not generated!");
	}
}

void UFGVoxelPlaceAbility::OnInputReleased(float TimeHeld)
{
	K2_EndAbilityLocally();
}

void UFGVoxelPlaceAbility::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow	ScopedPrediction(MyAbilityComponent);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
		}

		bool bIsTargetDataValid = LocalTargetDataHandle.IsValid(0);
		FGameplayAbilityTargetData_VoxelInfo* VoxelInfo = static_cast<FGameplayAbilityTargetData_VoxelInfo*>(LocalTargetDataHandle.Get(0));

#if WITH_SERVER_CODE
		if (AController* Controller = GetControllerFromActorInfo())
		{
			if (bIsTargetDataValid && Controller->HasAuthority())
			{
				// Perform some sanity over the sent target data...
				// modify bIsTargetDataValid accordingly
			}
		}
#endif //WITH_SERVER_CODE

		
		// Check if we have the materials to commit, if so, commit
		if (bIsTargetDataValid && CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			PlaceVoxel(VoxelInfo->GetEndPoint(), VoxelInfo->VoxelType);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Place ability %s failed to commit (bIsTargetDataValid=%d)"), *GetPathName(), bIsTargetDataValid ? 1 : 0);
			K2_EndAbility();
		}
	}

	// We've processed the data
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}
