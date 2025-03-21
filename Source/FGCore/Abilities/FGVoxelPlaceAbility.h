// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "AbilitySystem/Abilities/FGEquipmentAbility.h"
#include "FGVoxelPlaceAbility.generated.h"

/**
 * Place a voxel in the voxel grid at the first DDA hit.
 */
UCLASS()
class UFGVoxelPlaceAbility : public UFGEquipmentAbility
{
	GENERATED_BODY()
public:

	UFGVoxelPlaceAbility();

	//~ Begin Super
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End Super

	void PlaceVoxel(const FVector& Position, const int32 VoxelType);

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};
