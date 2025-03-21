// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#pragma once

#include "FGAbilityCost.h"
#include "ScalableFloat.h"

#include "FGAbilityCost_InventoryItem.generated.h"

class UFGGameplayAbility;
class UFGItemStaticData;

struct FGameplayAbilityActorInfo;
struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;
struct FGameplayTagContainer;

/**
 * UFGAbilityCost_InventoryItem
 *
 * Represents a cost that requires expending a quantity of an inventory item
 */
UCLASS(meta=(DisplayName="Inventory Item"))
class FGCORE_API UFGAbilityCost_InventoryItem : public UFGAbilityCost
{
	GENERATED_BODY()

public:
	UFGAbilityCost_InventoryItem();

	//~UFGAbilityCost interface
	virtual bool CheckCost(const UFGGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UFGGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UFGAbilityCost interface

protected:
	
	/** How much of the item to spend (keyed on ability level) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	FScalableFloat Quantity;

	/** Which item to consume */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	TObjectPtr<UFGItemStaticData> Item;
};