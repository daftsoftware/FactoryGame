// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Core/FGPawn.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "FGAbilityPawn.generated.h"

class UFGAbilitySystemComponent;
class UFGAbilitySet;
class UFGHealthSet;
class UFGCombatSet;
class UFGHealthComponent;
enum class EGameplayEffectReplicationMode : uint8;

UCLASS()
class AFGAbilityPawn : public AFGPawn, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:

    AFGAbilityPawn();

    //~AActor interface
    void PreInitializeComponents() override;
    void PostInitializeComponents() override;
    void NotifyControllerChanged() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~End of AActor interface

    UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
    UFGAbilitySystemComponent* GetFGAbilitySystemComponent() const;
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    void ResetAbilitySystem();
    void UninitializeAbilitySystem();

    //~IGameplayTagAssetInterface
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
    virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
    virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
    virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
    //~End of IGameplayTagAssetInterface

protected:

    // Begins the death sequence for the character (disables collision, disables movement, etc...)
    UFUNCTION()
    virtual void OnDeathStarted(AActor* OwningActor);

    // Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
    UFUNCTION()
    virtual void OnDeathFinished(AActor* OwningActor);

protected:

    UPROPERTY(EditDefaultsOnly)
    EGameplayEffectReplicationMode AbilitySystemReplicationMode;

    UPROPERTY(EditDefaultsOnly)
    TArray<TObjectPtr<UFGAbilitySet>> AbilitySets;

    UPROPERTY(Transient)
    TObjectPtr<UFGAbilitySystemComponent> AbilitySystemComponent = nullptr;

    // Health attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<UFGHealthSet> HealthSet;

    // Combat attribute set used by this actor.
    UPROPERTY()
    TObjectPtr<UFGCombatSet> CombatSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    TObjectPtr<UFGHealthComponent> HealthComponent;

};