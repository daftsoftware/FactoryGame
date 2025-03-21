// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "NiagaraActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "FGDebugDrawSubsystem.generated.h"

UCLASS()
class AFGDebugDrawFX final : public ANiagaraActor
{
	GENERATED_BODY()

	AFGDebugDrawFX(const FObjectInitializer& ObjectInitializer);
};

UCLASS()
class UFGDebugDrawSubsystem final : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:

	//~ Begin Super
	void Initialize(FSubsystemCollectionBase& Collection) override;
	bool ShouldCreateSubsystem(UObject* Outer) const override;
	void Tick(float DeltaTime) override;
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFGDebugDrawSubsystem, STATGROUP_Tickables)}
	//~ End Super

	UPROPERTY(Transient)
	TObjectPtr<AFGDebugDrawFX> DebugDrawFX;
};