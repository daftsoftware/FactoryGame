// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGAbilityPawn.h"

#include "AbilitySystem/FGAbilitySystemComponent.h"
#include "AbilitySystem/FGAbilitySet.h"
#include "Character/FGHealthComponent.h"
#include "AbilitySystem/Attributes/FGCombatSet.h"
#include "AbilitySystem/Attributes/FGHealthSet.h"
#include "FGGameplayTags.h"

AFGAbilityPawn::AFGAbilityPawn()
{
	NetCullDistanceSquared = 900000000.0f;
	AbilitySystemReplicationMode = EGameplayEffectReplicationMode::Minimal;

	// AbilitySystemComponent needs to be updated at a high frequency.
	NetUpdateFrequency = 100.0f;

	HealthComponent = CreateDefaultSubobject<UFGHealthComponent>(TEXT("HealthComponent"));
}

void AFGAbilityPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Create Ability System Component
	AbilitySystemComponent = NewObject<UFGAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AbilitySystemReplicationMode);
	AbilitySystemComponent->RegisterComponent();

	HealthSet = NewObject<UFGHealthSet>(this, TEXT("HealthSet"));
	CombatSet = NewObject<UFGCombatSet>(this, TEXT("CombatSet"));

	AbilitySystemComponent->SetNetAddressable();
	HealthSet->SetNetAddressable();
	CombatSet->SetNetAddressable();
}

void AFGAbilityPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (HasAuthority())
	{
		for (const UFGAbilitySet* AbilitySet : AbilitySets)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
			}
		}

		ForceNetUpdate();
	}
	
	// The attributes granted authoratively, wont be able to initialise predictively
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);
}

void AFGAbilityPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	
	if (!Controller)
	{
		return;
	}
	
	if (ensureAlwaysMsgf(AbilitySystemComponent, TEXT("The ASC should always be valid when this is called, otherwise harder initialization is required.")))
	{
		AbilitySystemComponent->RefreshAbilityActorInfo();
	}
}

void AFGAbilityPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

UFGAbilitySystemComponent* AFGAbilityPawn::GetFGAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* AFGAbilityPawn::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AFGAbilityPawn::ResetAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Reset ability system component
	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(TAG_ABILITY_BEHAVIOR_SURVIVESDEATH);

	AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
	AbilitySystemComponent->ClearAbilityInput();
	AbilitySystemComponent->RemoveAllGameplayCues();
}

void AFGAbilityPawn::UninitializeAbilitySystem()
{
	ResetAbilitySystem();

	// Clear ASC pointers for a neat-er destruction
	AbilitySystemComponent->ClearActorInfo();

	// Uninitialize hooked components that depend on the ASC
	HealthComponent->UninitializeFromAbilitySystem();

	AbilitySystemComponent = nullptr;
}

void AFGAbilityPawn::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UFGAbilitySystemComponent* FGASC = GetFGAbilitySystemComponent())
	{
		FGASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool AFGAbilityPawn::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const UFGAbilitySystemComponent* FGASC = GetFGAbilitySystemComponent())
	{
		return FGASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AFGAbilityPawn::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UFGAbilitySystemComponent* FGASC = GetFGAbilitySystemComponent())
	{
		return FGASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AFGAbilityPawn::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UFGAbilitySystemComponent* FGASC = GetFGAbilitySystemComponent())
	{
		return FGASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

void AFGAbilityPawn::OnDeathStarted(AActor* OwningActor)
{
	// TODO: We just died... temporarily disable collision etc for some time... death camera?
}

void AFGAbilityPawn::OnDeathFinished(AActor* OwningActor)
{
	ResetAbilitySystem();
	// TODO: Pool rest of the pawn...
}
