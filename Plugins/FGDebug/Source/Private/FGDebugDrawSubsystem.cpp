// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGDebugDrawSubsystem.h"

AFGDebugDrawFX::AFGDebugDrawFX(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bListedInSceneOutliner = false; // No setter weirdchamp, lol @ FSetActorHiddenInSceneOutliner???
#endif
}

void UFGDebugDrawSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |=	RF_Transient;
	SpawnParams.ObjectFlags &= ~RF_Transactional;
	
	DebugDrawFX = GetWorld()->SpawnActor<AFGDebugDrawFX>(SpawnParams);
}

bool UFGDebugDrawSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_ENABLE_DEBUG_DRAWING
	return Super::ShouldCreateSubsystem(Outer);
#else
	return false;
#endif
}

void UFGDebugDrawSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}