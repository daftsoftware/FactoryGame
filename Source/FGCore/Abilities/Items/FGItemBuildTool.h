// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Abilities/GameplayAbility.h"

#if 0 // Legacy build system - needs porting to GAS

#include "FGBuildTool.generated.h"

UCLASS()
class UFGBuildToolStartPlacingAbility : public UGameplayAbility, public FTickableGameObject
{
	GENERATED_BODY()
public:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void Tick(float DeltaTime) override;

	void ToggleGhost(AActor* Actor, bool NewValue);
	void StartPlacing();
	bool Place();

private:

	UPROPERTY(Transient)
	TObjectPtr<AFGBuildGridVisualizer> GridVisualizer;

	UPROPERTY(Transient)
	TObjectPtr<AFGBuildRotationVisualizer> RotationVisualizer;
	
	UPROPERTY(Transient)
	TObjectPtr<AActor> PlacingActor;

	FTransform	FastBuildInitialXForm;
	FVector		FastBuildDir			= FVector::ZeroVector;
	float		FastBuildCounter		= 0.f;
	float		FastBuildRate			= 0.5f;
	float		FastBuildMaxRate		= 0.1f;
	float		FastBuildAcceleration	= 0.1f;
	bool		bFastBuild				= false;
};

UCLASS()
class UFGBuildToolCancelPlacingAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
};

UCLASS()
class UFGBuildToolChangeDistanceAbility : public UGameplayAbility
{
	GENERATED_BODY()

	void ScrollBuildDistance(const FInputActionValue& Value);
	void ResetBuildDistance();
	void BuildRotateMode(const FInputActionValue& Value);
};

UCLASS()
class UFGBuildToolRotateAbility : public UGameplayAbility
{
	GENERATED_BODY()	
};

#endif