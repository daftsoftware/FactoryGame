// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Abilities/GameplayAbilityTargetActor.h"
#include "FGVoxelUtils.h"
#include "FGGameplayAbilityTargetActorDDA.generated.h"

UCLASS()
class AFGGameplayAbilityTargetActorDDA : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
public:

	AFGGameplayAbilityTargetActorDDA(const FObjectInitializer& ObjectInitializer);

	//~ Begin Super
	virtual void ConfirmTargetingAndContinue() override;
	//~ End Super

protected:
	
	virtual TOptional<FFGVoxelRayHit> PerformTrace(AActor* InSourceActor);
	
	FGameplayAbilityTargetDataHandle MakeTargetData(const FFGVoxelRayHit& HitResult) const;
};
