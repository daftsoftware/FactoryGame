// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGItemBackpack.h"
#include "Net/UnrealNetwork.h"
#include "Player/FGPlayerPawn.h" 
#include "Engine/StreamableManager.h"
#include "System/FGAssetManager.h"


UFGItemBackpack::UFGItemBackpack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UFGItemBackpack::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, EquipmentList);
}

void UFGItemBackpack::InitializeBackpack()
{
	// Note: We cannot initialise the backpack from the backpack constructor or postinit itself 
	// since when the player spawns it messes up the pointer of the backpack
	// so we initialise it from the player, whenever it calls its constructor.
	if (!IsTemplate())
	{
		EquipmentList.SetOwnerBackpack(this);
	}
}

void UFGItemBackpack::Equip(int32 StackIndex)
{
	UE_LOG(LogTemp, Display, TEXT("Equipped item"));

	if (HasAuthority() && GetPawn()->GetFGAbilitySystemComponent())	// Grant item abilities.
	{
		TArray<FSoftObjectPath> AbilitySetsToLoad;

		checkf(GetItemStacks()[StackIndex].StaticData, TEXT("Item type is null!"));

		Algo::Transform(GetItemStacks()[StackIndex].StaticData->AbilitySets, AbilitySetsToLoad, [](TSoftObjectPtr<UFGAbilitySet> AbilitySet)
			{
				return AbilitySet.ToSoftObjectPath();
			});

		FStreamableManager& StreamableMgr = UFGAssetManager::GetStreamableManager();

		StreamableMgr.RequestAsyncLoad(AbilitySetsToLoad, FStreamableDelegate::CreateWeakLambda(this,
			[this, StackIndex]()
			{
				UFGEquipmentSlot* Result = EquipmentList.AddEntry(StackIndex);
				if (Result != nullptr)
				{
					// TODO: Result->OnEquipped();
					if (GetPawn()->IsUsingRegisteredSubObjectList())
					{
						GetPawn()->AddReplicatedSubObject(Result);
					}
				}
			}));
	}
}

void UFGItemBackpack::UnEquip(int32 StackIndex)
{
	UE_LOG(LogTemp, Display, TEXT("Unequipped item"));

	if (!HasAuthority())
	{
		return;
	}

	UFGEquipmentSlot* Result = EquipmentList.RemoveEntry(StackIndex);

	if (Result != nullptr)
	{
		// TODO: Do post-process server stuff with this equipment
		if (GetPawn()->IsUsingRegisteredSubObjectList())
		{
			GetPawn()->RemoveReplicatedSubObject(Result);
		}

		// TODO: Result->OnUnequipped();
	}
}

void UFGItemBackpack::OnIndexPreChange(int32 Index)
{
	if (GetPawn()->QuickItemIndex == Index && ItemStacks[Index].StaticData)
	{
		UnEquip(Index);
	}
}

void UFGItemBackpack::OnIndexPostChange(int32 Index)
{
	if (GetPawn()->QuickItemIndex == Index && ItemStacks[Index].StaticData)
	{
		Equip(Index);
	}
}

void FFGEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		const FFGAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.EquipmentSlot != nullptr)
		{
			// TODO: Entry.EquipmentSlot->OnUnequipped();
		}
	}
}

void FFGEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FFGAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.EquipmentSlot != nullptr)
		{
			// TODO: Entry.EquipmentSlot->OnEquipped();
		}
	}
}

UFGEquipmentSlot* FFGEquipmentList::AddEntry(int32 StackIndex)
{
	UFGEquipmentSlot* NewEquipSlot = nullptr;

	check(IsInGameThread());
	check(OwnerBackpack);
	check(OwnerBackpack->HasAuthority());

	// @TODO: Check stack index is as we expect here, if it's not early out.
	const FFGItemStack& ItemStack = OwnerBackpack->GetItemStacks()[StackIndex];

	FFGAppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.StackIndex = StackIndex;
	NewEntry.EquipmentSlot = NewObject<UFGEquipmentSlot>(OwnerBackpack->GetPawn());

	NewEquipSlot = NewEntry.EquipmentSlot;
	NewEquipSlot->BackpackIndex = StackIndex;

	if (UFGAbilitySystemComponent* ASC = GetFGAbilitySystemComponent())
	{
		for (TSoftObjectPtr<UFGAbilitySet> AbilitySet : ItemStack.StaticData->AbilitySets)
		{
			AbilitySet.Get()->GiveToAbilitySystem(ASC, &NewEntry.GrantedHandles, NewEquipSlot);
		}
	}

	// TODO: Do server side things with the equipment

	MarkItemDirty(NewEntry);

	return NewEquipSlot;
}

UFGEquipmentSlot* FFGEquipmentList::RemoveEntry(int32 StackIndex)
{
	UFGEquipmentSlot* NewEquipSlot = nullptr;

	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FFGAppliedEquipmentEntry& Entry = *EntryIt;
		if (Entry.StackIndex == StackIndex)
		{
			if (UFGAbilitySystemComponent* ASC = GetFGAbilitySystemComponent())
			{
				Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
			}

			// TODO: Do server side things with the equipment
			NewEquipSlot = Entry.EquipmentSlot;

			EntryIt.RemoveCurrent();
			MarkArrayDirty();

			break;
		}
	}

	return NewEquipSlot;
}

UFGAbilitySystemComponent* FFGEquipmentList::GetFGAbilitySystemComponent() const
{
	check(OwnerBackpack);
	check(OwnerBackpack->GetPawn());
	return OwnerBackpack->GetPawn()->GetFGAbilitySystemComponent();
}

FString FFGAppliedEquipmentEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s"), *GetNameSafe(EquipmentSlot));
}
