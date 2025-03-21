// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPlayerContainer.h"
#include "Player/FGPlayerPawn.h" 


UFGPlayerContainer::UFGPlayerContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

// We can override this on inventory for example to only replicate to the owning player.
ELifetimeCondition UFGPlayerContainer::GetReplicationCondition() const
{
	return ELifetimeCondition::COND_OwnerOnly;
}

bool UFGPlayerContainer::ShouldReplicate() const
{
	return true;
}

AFGPlayerPawn* UFGPlayerContainer::GetPawn() const
{
	return CastChecked<AFGPlayerPawn>(GetOuter());
}