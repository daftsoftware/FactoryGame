// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "World/FGVoxelCollisionManager.h"
#include "FGPlayerCollisionManager.generated.h"

class UBoxComponent;

UCLASS()
class AFGPlayerCollisionManager final : public AFGVoxelCollisionManager
{
	GENERATED_BODY()
public:

	AFGPlayerCollisionManager();

	//~ Begin Super
	void Tick(float DeltaSeconds) override;
	//~ End Super

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBoxComponent>> BoxComponentPool;
};