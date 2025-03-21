// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGPlayerContainer.h"
#include "FGItemBackpack.generated.h"

class AFGPlayerPawn;

/** A single piece of applied equipment */
USTRUCT(BlueprintType)
struct FFGAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FFGAppliedEquipmentEntry()
	{}

	FString GetDebugString() const;

private:
	friend FFGEquipmentList;
	friend UFGItemBackpack;

	// The equipment class that got equipped
	UPROPERTY()
	int32 StackIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<UFGEquipmentSlot> EquipmentSlot = nullptr;

	// Authority-only list of granted handles
	UPROPERTY(NotReplicated)
	FFGAbilitySet_GrantedHandles GrantedHandles;
};

/** List of applied equipment */
USTRUCT(BlueprintType)
struct FFGEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	FFGEquipmentList()
		: OwnerBackpack(nullptr)
	{
	}

public:
	
	void SetOwnerBackpack(UFGItemBackpack* InOwnerBackpack)
	{
		OwnerBackpack = InOwnerBackpack;
	}

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFGAppliedEquipmentEntry, FFGEquipmentList>(Entries, DeltaParms, *this);
	}

	UFGEquipmentSlot* AddEntry(int32 StackIndex);
	UFGEquipmentSlot* RemoveEntry(int32 StackIndex);

private:
	UFGAbilitySystemComponent* GetFGAbilitySystemComponent() const;

	friend UFGItemBackpack;

private:
	// Replicated list of equipment entries
	UPROPERTY()
	TArray<FFGAppliedEquipmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UFGItemBackpack> OwnerBackpack;
};

template<>
struct TStructOpsTypeTraits<FFGEquipmentList> : public TStructOpsTypeTraitsBase2<FFGEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(NotBlueprintable)
class UFGItemBackpack : public UFGPlayerContainer
{
	GENERATED_BODY()

public:
	
	UFGItemBackpack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeBackpack();
	void Equip(int32 StackIndex);
	void UnEquip(int32 StackIndex);
	virtual void OnIndexPreChange(int32 Index) override;
	virtual void OnIndexPostChange(int32 Index) override;

	TMulticastDelegate<void(int32)> OnEquipmentChanged;

protected:

	UPROPERTY(Replicated)
	FFGEquipmentList EquipmentList;
};