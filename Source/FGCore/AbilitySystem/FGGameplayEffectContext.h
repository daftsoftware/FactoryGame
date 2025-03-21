// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#pragma once

#include "GameplayEffectTypes.h"

#include "FGGameplayEffectContext.generated.h"

class AActor;
class FArchive;
class IFGAbilitySourceInterface;
class UObject;
class UPhysicalMaterial;

USTRUCT()
struct FFGGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FFGGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	FFGGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	/** Returns the wrapped FFGGameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static FGCORE_API FFGGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	/** Sets the object used as the ability source */
	void SetAbilitySource(const IFGAbilitySourceInterface* InObject, float InSourceLevel);

	/** Returns the ability source interface associated with the source object. Only valid on the authority. */
	const IFGAbilitySourceInterface* GetAbilitySource() const;

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FFGGameplayEffectContext* NewContext = new FFGGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FFGGameplayEffectContext::StaticStruct();
	}

	/** Overridden to serialize new fields */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	/** Returns the physical material from the hit result if there is one */
	const UPhysicalMaterial* GetPhysicalMaterial() const;

protected:
	/** Ability Source object (should implement IFGAbilitySourceInterface). NOT replicated currently */
	UPROPERTY()
	TWeakObjectPtr<const UObject> AbilitySourceObject;
};

template<>
struct TStructOpsTypeTraits<FFGGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FFGGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};