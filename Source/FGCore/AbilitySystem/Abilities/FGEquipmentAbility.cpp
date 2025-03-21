// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#include "FGEquipmentAbility.h"
#include "Inventory/FGItemContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGEquipmentAbility)

UFGEquipmentAbility::UFGEquipmentAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFGEquipmentSlot* UFGEquipmentAbility::GetEquipmentSlot() const
{
	if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
	{
		return CastChecked<UFGEquipmentSlot>(Spec->SourceObject.Get());
	}

	return nullptr;
}