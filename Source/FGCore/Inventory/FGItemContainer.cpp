// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGItemContainer.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

void UFGEquipmentSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BackpackIndex);
}

void UFGItemContainer::GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const
{
	DOREPDYNAMICCONDITION_INITCONDITION_FAST(ThisClass, ItemStacks, GetReplicationCondition());
	if(GetReplicationCondition() == COND_Custom)
	{
		DOREPCUSTOMCONDITION_ACTIVE_FAST(ThisClass, ItemStacks, ShouldReplicate());
	}
}

void UFGItemContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_Dynamic;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(UFGItemContainer, ItemStacks, Params);
}

void UFGItemContainer::PostInitProperties()
{
	Super::PostInitProperties();

	if(IsTemplate())
	{
		return;
	}

	// @TODO: We can remove this by just using the FFGVoxelNetRef as the stable network name
	// predicting on the client the creation of the UObject on the entity system when we enter
	// a chunk with a chest in it.
	
	UWorld* World = GEngine->GetCurrentPlayWorld();

	// DOP - 25/05/24 - We don't want to do this anymore to stay compliant with old rep sys.
#if 0
	if(World && World->IsGameWorld() && World->GetNetMode() == NM_Client)
	{
		if(ShouldRegisterToGlobalStorage())
		{
			auto* EntitySys = World->GetSubsystem<UFGEntitySystem>();
			EntitySys->Containers.Add(this);
		}
	}
	else
	{
		UE_LOGFMT(LogTemp, Error, "Failed to GetWorld() in UFGItemContainer::PostInitProperties()");
	}
#endif
}

#if UE_WITH_IRIS

void UFGItemContainer::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

// We can override this on inventory for example to only replicate to the owning player.
ELifetimeCondition UFGItemContainer::GetReplicationCondition() const
{
	return ELifetimeCondition::COND_Custom;
}

bool UFGItemContainer::ShouldReplicate() const
{
	return true;// ViewingPlayers.Num() > 0;
}

int32 UFGItemContainer::AutoStack(const FFGItemStack Item)
{
	// Check for existing stacks first.
	for(int32 Stack = 0; Stack < ItemStacks.Num(); Stack++)
	{
		int32 CombineStackSize = ItemStacks[Stack].StackSize + Item.StackSize;
		
		if(ItemStacks[Stack].StaticData == Item.StaticData
			&&  CombineStackSize < ItemStacks[Stack].StaticData->MaxStackSize)
		{
			return Stack;
		}
	}

	// Check for empty slots if we didn't find a stackable.
	for(int32 Slot = 0; Slot < ItemStacks.Num(); Slot++)
	{
		if(ItemStacks[Slot].StaticData == nullptr)
		{
			return Slot;
		}
	}
	return INDEX_NONE;
}

void UFGItemContainer::OpenContainer(APlayerState* PlayerState)
{
	if (!HasAuthority())
	{
		return;
	}

	ViewingPlayers.Add(PlayerState);
}

void UFGItemContainer::CloseContainer(APlayerState* PlayerState)
{
	if (!HasAuthority())
	{
		return;
	}

	ViewingPlayers.Remove(PlayerState);
}

void UFGItemContainer::MoveStack(int32 FromSlot, int32 ToSlot, UFGItemContainer* TargetContainer)
{
	if (!TargetContainer || TargetContainer == this)
	{
		if (FromSlot == ToSlot) 
		{ 
			return; 
		}
		
		OnIndexPreChange(FromSlot);

		TransferToSlot(ToSlot, ItemStacks[FromSlot]);
		ItemStacks[FromSlot] = FFGItemStack();

		OnIndexPostChange(FromSlot);
	}
	else
	{
		OnIndexPreChange(FromSlot);

		TargetContainer->TransferToSlot(ToSlot, ItemStacks[FromSlot]);
		ItemStacks[FromSlot] = FFGItemStack();
		
		OnIndexPostChange(FromSlot);

		// We need to mark ItemStacks dirty here as the TargetContainer != this.
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStacks, this);
	}
}

void UFGItemContainer::TransferToSlot(int32 ToSlot, const FFGItemStack& Stack, int32 Amount)
{
	if (Stack.StaticData == ItemStacks[ToSlot].StaticData) // Stack Combine.
	{
		ItemStacks[ToSlot].StackSize += ((Amount == -1) ? Stack.StackSize : Amount);
	}
	else // Move Stack.
	{
		OnIndexPreChange(ToSlot);

		ItemStacks[ToSlot] = Stack;
		if (Amount != -1)
		{
			ItemStacks[ToSlot].StackSize = Amount;
		}

		OnIndexPostChange(ToSlot);
	}
	
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStacks, this);
}

void UFGItemContainer::SplitStack(int32 FromSlot, int32 ToSlot, int32 Amount, UFGItemContainer* TargetContainer)
{
	checkf(Amount != INDEX_NONE, TEXT("Invalid split amount."));

	if (!TargetContainer || TargetContainer == this)
	{
		if (FromSlot == ToSlot)
		{
			return;
		}

		if (ItemStacks[FromSlot].StackSize == 1) // Single stack, just move instead.
		{
			MoveStack(FromSlot, ToSlot, TargetContainer);
		}
		else // Stack Combine.
		{
			TransferToSlot(ToSlot, ItemStacks[FromSlot], Amount);
			ItemStacks[FromSlot].StackSize -= Amount;
		}	
	}
	else
	{
		if (ItemStacks[FromSlot].StackSize == 1) // Single stack, just move instead.
		{
			MoveStack(FromSlot, ToSlot, TargetContainer);
		}
		else // Stack Combine.
		{
			TargetContainer->TransferToSlot(ToSlot, ItemStacks[FromSlot], Amount);
			ItemStacks[FromSlot].StackSize -= Amount;
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStacks, this);
		}
	}
}

bool UFGItemContainer::IsValidSlot(int32 Slot) const
{
	return ItemStacks.IsValidIndex(Slot) && ItemStacks[Slot].StaticData;
}

bool UFGItemContainer::HasAuthority() const
{
	const AActor* ActorOwner = CastChecked<AActor>(GetOuter());
	return ActorOwner->HasAuthority();
}

void UFGItemContainer::SetItemStackElement(int32 Index, const FFGItemStack& Item)
{
	if (!ItemStacks.IsValidIndex(Index))
	{
		return;
	}
	
	OnIndexPreChange(Index);

	ItemStacks[Index] = Item;

	OnIndexPostChange(Index);

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStacks, this);
}

int32 UFGItemContainer::ConsumeItem(UFGItemStaticData* Item, int32 Amount)
{
	int32 TotalConsumed = 0;
	for (int32 Stack = 0; Stack < ItemStacks.Num() && TotalConsumed < Amount; Stack++)
	{
		if (ItemStacks[Stack].StaticData == Item)
		{
			const int32 SubstractedCount = ConsumeItemIndex(Amount - TotalConsumed);
			TotalConsumed += SubstractedCount;
		}
	}

	return TotalConsumed;
}

int32 UFGItemContainer::ConsumeItemIndex(int32 Index, int32 Amount)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStacks, this);
	
	const int32 SubstractedCount = FMath::Min(Amount, ItemStacks[Index].StackSize);
	ItemStacks[Index].StackSize -= SubstractedCount;
	if (ItemStacks[Index].StackSize == 0)
	{
		OnIndexPreChange(Index);
		ItemStacks[Index] = FFGItemStack();
		OnIndexPostChange(Index);
	}
	return SubstractedCount;
}

int32 UFGItemContainer::GetItemCount(UFGItemStaticData* Item)
{
	// Check for existing stacks first.
	int32 TotalConsumed = 0;
	for (int32 Stack = 0; Stack < ItemStacks.Num(); Stack++)
	{
		if (ItemStacks[Stack].StaticData == Item)
		{
			TotalConsumed += ItemStacks[Stack].StackSize;
		}
	}

	return TotalConsumed;
}

TArray<FFGItemStack>& UFGItemContainer::GetItemStacks_Mutable()
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStacks, this);
	return ItemStacks;
}

void UFGItemContainer::Client_SendDelta_Implementation(const TArray<FFGItemStack>& Delta)
{
	// stub
}
