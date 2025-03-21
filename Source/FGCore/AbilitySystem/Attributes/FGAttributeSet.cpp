// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#include "FGAttributeSet.h"

#include "AbilitySystem/FGAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGAttributeSet)

class UWorld;


UFGAttributeSet::UFGAttributeSet()
{
}

UWorld* UFGAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UFGAbilitySystemComponent* UFGAttributeSet::GetFGAbilitySystemComponent() const
{
	return Cast<UFGAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

