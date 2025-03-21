// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#pragma once

#include "FGGameplayAbility.h"

#include "FGEquipmentAbility.generated.h"

class UFGEquipmentSlot;

/**
 * UFGEquipmentAbility
 *
 *	An ability granted by and associated with an equipment instance
 */
UCLASS(HideCategories = Input, Meta = (ShortTooltip = "An ability granted by and associated with an equipment instance."))
class FGCORE_API UFGEquipmentAbility : public UFGGameplayAbility
{
	GENERATED_BODY()

public:

	UFGEquipmentAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "FG|Ability")
	UFGEquipmentSlot* GetEquipmentSlot() const;
};