// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "FGGameplayAbilityTargetDataVoxelType.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_VoxelInfo : public FGameplayAbilityTargetData_LocationInfo
{
	GENERATED_BODY()

	FGameplayAbilityTargetData_VoxelInfo()
		: VoxelType(INDEX_NONE)
	{ }

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Targeting)
	int32 VoxelType;
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_VoxelInfo::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_VoxelInfo> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_VoxelInfo>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};
