// Copyright Epic Games, Inc. All Rights Reserved.

#include "FGAbilityCost_HeldItem.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"
#include "FGEquipmentAbility.h"
#include "Inventory/FGItemContainer.h"
#include "Player/FGPlayerPawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGAbilityCost_HeldItem)

UFGAbilityCost_HeldItem::UFGAbilityCost_HeldItem()
{
	Quantity.SetValue(1.0f);
}

bool UFGAbilityCost_HeldItem::CheckCost(const UFGGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (const UFGEquipmentAbility* EquipmentAbility = Cast<const UFGEquipmentAbility>(Ability))
	{
		if (UFGEquipmentSlot* ItemInstance = EquipmentAbility->GetEquipmentSlot())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumItemsToConsumeReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);

			const FFGItemStack* ItemStack = CastChecked<AFGPlayerPawn>(Ability->GetAvatarActorFromActorInfo())->GetItemStack(ItemInstance->BackpackIndex);

			return ItemStack->StackSize >= NumItemsToConsume;
		}
	}

	return false;
}

void UFGAbilityCost_HeldItem::ApplyCost(const UFGGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (const UFGEquipmentAbility* EquipmentAbility = Cast<const UFGEquipmentAbility>(Ability))
		{
			if (UFGEquipmentSlot* ItemInstance = EquipmentAbility->GetEquipmentSlot())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumItemsToConsumeReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);
				
				UFGItemBackpack& Backpack = CastChecked<AFGPlayerPawn>(Ability->GetAvatarActorFromActorInfo())->GetBackpack();
				Backpack.ConsumeItemIndex(ItemInstance->BackpackIndex, NumItemsToConsume);
			}
		}
	}
}