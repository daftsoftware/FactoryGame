// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#include "FGCombatSet.h"

#include "AbilitySystem/Attributes/FGAttributeSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGCombatSet)

class FLifetimeProperty;


UFGCombatSet::UFGCombatSet()
	: BaseDamage(0.0f)
	, BaseHeal(0.0f)
{
}

void UFGCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UFGCombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFGCombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

void UFGCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFGCombatSet, BaseDamage, OldValue);
}

void UFGCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFGCombatSet, BaseHeal, OldValue);
}

