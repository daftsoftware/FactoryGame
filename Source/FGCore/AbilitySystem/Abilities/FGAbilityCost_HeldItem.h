// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#pragma once

#include "FGAbilityCost.h"
#include "ScalableFloat.h"

#include "FGAbilityCost_HeldItem.generated.h"

class UFGGameplayAbility;
class UFGItemStaticData;

struct FGameplayAbilityActorInfo;
struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;
struct FGameplayTagContainer;

/**
 * UFGAbilityCost_HeldItem
 *
 * Represents a cost that requires expending a quantity of an inventory item
 */
UCLASS(meta=(DisplayName="Held Item"))
class FGCORE_API UFGAbilityCost_HeldItem : public UFGAbilityCost
{
	GENERATED_BODY()

public:
	UFGAbilityCost_HeldItem();

	//~UFGAbilityCost interface
	virtual bool CheckCost(const UFGGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UFGGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UFGAbilityCost interface

protected:
	
	/** How much of the item to spend (keyed on ability level) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	FScalableFloat Quantity;
};