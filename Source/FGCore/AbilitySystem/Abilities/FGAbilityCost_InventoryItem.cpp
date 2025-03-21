// Copyright Epic Games, Inc. All Rights Reserved.

#include "FGAbilityCost_InventoryItem.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"
#include "FGGameplayAbility.h"
#include "Player/FGPlayerPawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGAbilityCost_InventoryItem)

UFGAbilityCost_InventoryItem::UFGAbilityCost_InventoryItem()
{
	Quantity.SetValue(1.0f);
}

bool UFGAbilityCost_InventoryItem::CheckCost(const UFGGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AFGPlayerPawn* Pawn = CastChecked<AFGPlayerPawn>(Ability->GetAvatarActorFromActorInfo()))
	{
		if (UFGItemBackpack* Backpack = Pawn->Backpack)
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumItemsToConsumeReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);

			return Pawn->Backpack->GetItemCount(Item) >= NumItemsToConsume;
		}
	}
	return false;
}

void UFGAbilityCost_InventoryItem::ApplyCost(const UFGGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (AFGPlayerPawn* Pawn = CastChecked<AFGPlayerPawn>(Ability->GetAvatarActorFromActorInfo()))
		{
			if (UFGItemBackpack* Backpack = Pawn->Backpack)
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumItemsToConsumeReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);

				Pawn->Backpack->ConsumeItem(Item, NumItemsToConsume);
			}
		}
	}
}