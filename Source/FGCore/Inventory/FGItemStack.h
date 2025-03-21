// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGItemAttributeFloat.h"
#include "FGItemAttributeInt.h"
#include "AbilitySystem/FGAbilitySet.h"
#include "GameplayEffectTypes.h"
#include "InstancedStructContainer.h"
#include "Containers/Array.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "FGItemStack.generated.h"

class UFGAbilitySet;
class UGameplayAbility;
class UGameplayEffect;

USTRUCT()
struct FFGItemAttributes
{
	GENERATED_BODY()

	UPROPERTY()
	FFGItemAttributeStackInt IntegerAttributes;

	UPROPERTY()
	FFGItemAttributeStackFloat FloatAttributes;
};

/*
	FG item static data is the data asset that holds
	the static data values of items or equipment and
	defines the main configuration for any given item
	and which types of runtime data it contains.
*/
UCLASS()
class FGCORE_API UFGItemStaticData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	UFGItemStaticData();

	//~ Begin Super
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~ End Super

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSlateBrush Icon;
	
	UPROPERTY(EditDefaultsOnly)
	int32 MaxStackSize = 64;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSoftObjectPtr<UFGAbilitySet>> AbilitySets;
	
	UPROPERTY(EditDefaultsOnly)
	TOptional<TObjectPtr<UTexture2D>> HeldTexture;

	UPROPERTY(EditDefaultsOnly)
	float HeldThickness = 2;
	
	UPROPERTY(EditDefaultsOnly)
	float HeldVoxelSize = 4.0;
};

/*
  	Item stacks represent a resizable transient data stack
  	of item indexes - which represents an item type.
  	
  	Item runtime data is PER-STACK, meaning that you can't
  	stack items with unique attributes; without discarding
  	all attributes from any "other stack" combined with the
  	root stack. Item stacks themselves derive from instanced
  	struct container for this reason, so that you may compose
  	them with various other runtime struct types.
*/
USTRUCT(BlueprintType)
struct FFGItemStack
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UFGItemStaticData> StaticData;

	UPROPERTY()
	TOptional<FFGItemAttributes> RuntimeData;

	UPROPERTY(BlueprintReadWrite, Meta = (ClampMin = 0, ClampMax = 65535, UIMin = 0, UIMax = 65535))
	int32 StackSize = 1;

	FFGItemStack(UFGItemStaticData* InItemType = nullptr, int32 InStackSize = 1)
		: StaticData(InItemType), StackSize(InStackSize)
	{}	
};
