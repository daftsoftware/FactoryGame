// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "FGTransportPipeManager.generated.h"

class AFGTransportPipe;

UCLASS()
class UFGTransportPipeManager final : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:

	//~ Begin Super
	void Tick(float DeltaTime) override;
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFGTransportPipeManager, STATGROUP_Tickables) }
	//~ End Super

	void RegisterNode(AFGTransportPipe* NewNode);
	void UnregisterNode(AFGTransportPipe* OldNode);

	TMap<FIntVector, AFGTransportPipe*> NodeMap;
};
