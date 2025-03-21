// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#include "FGGameplayEffectContext.h"

#include "AbilitySystem/FGAbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGGameplayEffectContext)

class FArchive;

FFGGameplayEffectContext* FFGGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FFGGameplayEffectContext::StaticStruct()))
	{
		return (FFGGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

bool FFGGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);
	return true;
}

#if UE_WITH_IRIS
namespace UE::Net
{
	// Forward to FGameplayEffectContextNetSerializer
	// Note: If FFGGameplayEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(FGGameplayEffectContext, FGameplayEffectContextNetSerializer);
}
#endif

void FFGGameplayEffectContext::SetAbilitySource(const IFGAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

const IFGAbilitySourceInterface* FFGGameplayEffectContext::GetAbilitySource() const
{
	return Cast<IFGAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FFGGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}