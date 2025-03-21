// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelRemoveAbility.h"

#include "AbilitySystemComponent.h"
#include "System/FGAssetManager.h"
#include "System/FGGameData.h"
#include "World/FGVoxelSystem.h"
#include "FGVoxelDefines.h"
#include "FGVoxelUtils.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilitySystem/FGGameplayAbilityTargetActorDDA.h"
#include "Engine/StreamableManager.h"
#include "Inventory/FGItemSubsystem.h"
#include "Inventory/FGVoxelItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

UFGVoxelRemoveAbility::UFGVoxelRemoveAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UFGVoxelRemoveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilityTask_WaitInputRelease* WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	WaitInputRelease->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
	WaitInputRelease->ReadyForActivation();

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

			FVector End = RayHit.GetValue().VoxelCenter;

			// Fill out the target data from the hit results
			FGameplayAbilityTargetData_LocationInfo* VoxelTypeTargetData = new FGameplayAbilityTargetData_LocationInfo();
			VoxelTypeTargetData->TargetLocation.LiteralTransform = FTransform(End);

			FGameplayAbilityTargetDataHandle TargetData;
			TargetData.Add(VoxelTypeTargetData);

			// Process the target data immediately
			OnTargetDataReadyCallback(TargetData, FGameplayTag());
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UFGVoxelRemoveAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
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

void UFGVoxelRemoveAbility::RemoveVoxel(const FVector& Position)
{
	auto* VoxelSys = GetWorld()->GetSubsystem<UFGVoxelSystem>();
	auto& VoxelGrid = VoxelSys->VoxelGrid;

	FIntVector ChunkCoordinate = UFGVoxelUtils::VectorToChunkCoord(Position);
	FIntVector VoxelCoordinate = UFGVoxelUtils::VectorToVoxelCoord(Position);

	if (VoxelGrid->IsChunkGenerated(ChunkCoordinate))
	{
		// Add the voxel item to the player's inventory.
		if (GetControllerFromActorInfo()->HasAuthority())
		{
			FFGChunkHandle ChunkDataHandle = VoxelGrid->FindChunkChecked(ChunkCoordinate);
			int32 OldVoxelType = VoxelGrid->GetChunkDataSafe(ChunkDataHandle)->GetVoxel(VoxelCoordinate);

			TMultiMap<FName, FString> TagValues;

			const FGameplayTag* VoxelTag = GVoxelTypeMap.FindKey(OldVoxelType);
			checkf(VoxelTag, TEXT("Voxel type not found in tag map!"));

			TagValues.Add("VoxelTag", VoxelTag->GetTagName().ToString());

			TArray<FAssetData> FoundAssets;
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			AssetRegistryModule.Get().GetAssetsByTagValues(TagValues, FoundAssets);

			bool FoundItem = false;
			
			for(FAssetData& AssetData : FoundAssets)
			{
				if(AssetData.GetClass() == UFGVoxelItemData::StaticClass())
				{
					FoundItem = true;
					
					TSoftObjectPtr<UFGVoxelItemData> ItemDataToGive = AssetData.GetAsset();

					auto* ItemSubsystem = GetWorld()->GetSubsystem<UFGItemSubsystem>();
					ItemSubsystem->GiveItem(GetFGPlayerControllerFromActorInfo(), ItemDataToGive);
				}
			}

			checkf(FoundItem, TEXT("No item assets found for voxel! Make sure it has the correct tag!"));
		}

		VoxelSys->ModifyVoxel(ChunkCoordinate, VoxelCoordinate, VOXELTYPE_NONE);

		TSoftObjectPtr<> DigSound = UFGGameData::Get().DigSFX;
		FStreamableManager& StreamableMgr = UFGAssetManager::GetStreamableManager();

		// Play dig SFX.
		StreamableMgr.RequestAsyncLoad(DigSound.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, Position, DigSound]()
			{
				check(IsInGameThread());

				auto* LoadedSound = CastChecked<USoundBase>(DigSound.Get());
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), LoadedSound, Position);
			}));
	}
	else
	{
		UE_LOGFMT(LogTemp, Error, "Tried to modify voxel when chunk was not generated!");
	}
}

void UFGVoxelRemoveAbility::OnInputReleased(float TimeHeld)
{
	K2_EndAbilityLocally();
}

void UFGVoxelRemoveAbility::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
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
		FGameplayAbilityTargetData_LocationInfo* VoxelInfo = static_cast<FGameplayAbilityTargetData_LocationInfo*>(LocalTargetDataHandle.Get(0));

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
			RemoveVoxel(VoxelInfo->GetEndPoint());
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