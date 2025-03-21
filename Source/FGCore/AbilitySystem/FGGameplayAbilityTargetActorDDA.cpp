// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGGameplayAbilityTargetActorDDA.h"

#include "FGVoxelUtils.h"

AFGGameplayAbilityTargetActorDDA::AFGGameplayAbilityTargetActorDDA(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void AFGGameplayAbilityTargetActorDDA::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		TOptional<FFGVoxelRayHit> OutRayHit = PerformTrace(SourceActor);
		
		if(OutRayHit.IsSet()) // Hit valid voxel
		{
			FGameplayAbilityTargetDataHandle Handle = MakeTargetData(PerformTrace(SourceActor).GetValue());
			TargetDataReadyDelegate.Broadcast(Handle);
		}
	}
}

TOptional<FFGVoxelRayHit> AFGGameplayAbilityTargetActorDDA::PerformTrace(AActor* InSourceActor)
{
	const FVector Start = StartLocation.GetTargetingTransform().GetLocation();
	const FVector XDir = FRotationMatrix(StartLocation.GetTargetingTransform().Rotator()).GetScaledAxis(EAxis::X);
	const FVector End = Start + XDir * 350.0f;
	
	return UFGVoxelUtils::RayVoxelIntersection(
		GetWorld(),
		Start,
		End);
}

FGameplayAbilityTargetDataHandle AFGGameplayAbilityTargetActorDDA::MakeTargetData(const FFGVoxelRayHit& HitResult) const
{
	FTransform Start = FTransform(FRotator::ZeroRotator, HitResult.Start, FVector::OneVector);
	FTransform End = FTransform(FRotator::ZeroRotator, HitResult.VoxelCenter, FVector::OneVector);
	
	FGameplayAbilityTargetData_LocationInfo* NewTargetData = new FGameplayAbilityTargetData_LocationInfo();
	NewTargetData->SourceLocation.LiteralTransform = Start;
	NewTargetData->TargetLocation.LiteralTransform = End;
	return FGameplayAbilityTargetDataHandle(NewTargetData);
}