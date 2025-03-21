// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#pragma once

#include "UObject/Interface.h"

#include "FGAbilitySourceInterface.generated.h"

class UObject;
class UPhysicalMaterial;
struct FGameplayTagContainer;

/** Base interface for anything acting as a ability calculation source */
UINTERFACE()
class UFGAbilitySourceInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IFGAbilitySourceInterface
{
	GENERATED_IINTERFACE_BODY()

	// TODO: --
};