// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGItemContainer.h"
#include "FGPlayerContainer.generated.h"

class AFGPlayerPawn;

UCLASS(NotBlueprintable)
class UFGPlayerContainer : public UFGItemContainer
{
	GENERATED_BODY()

public:
	
	UFGPlayerContainer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/*
		Returns which ELifetimeCondition should be used for this container
		to replicate to relevant connections. This can be used to change the
		condition if the property is set to use COND_Dynamic in an object's
		GetLifetimeReplicatedProps implementation.
	*/
	virtual ELifetimeCondition GetReplicationCondition() const override;
	virtual bool ShouldReplicate() const override;
	
	AFGPlayerPawn* GetPawn() const;
};