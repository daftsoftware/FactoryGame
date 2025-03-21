// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "AbilitySystem/Abilities/FGGameplayAbility.h"
#include "FGVoxelRemoveAbility.generated.h"

/**
 * Remove a voxel in the voxel grid at the first DDA hit.
 */
UCLASS()
class UFGVoxelRemoveAbility : public UFGGameplayAbility
{
	GENERATED_BODY()
public:

	UFGVoxelRemoveAbility();

	//~ Begin Super
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End Super

	void RemoveVoxel(const FVector& Position);

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};
