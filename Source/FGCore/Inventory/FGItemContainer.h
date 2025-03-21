// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGItemStack.h"
#include "Containers/Array.h"
#include "FGItemContainer.generated.h"

/**
 * Network addressable equipment slot used for gameplay abilities.
 */
UCLASS(NotBlueprintable)
class UFGEquipmentSlot : public UObject
{
	GENERATED_BODY()
public:

	//~ Begin Super
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	virtual UWorld* GetWorld() const override
	{
		return GetOuter()->GetWorld();	
	}
	
	//~ End Super
	UPROPERTY(Replicated)
	int32 BackpackIndex;
};

/*
	FG item container is the main storage structure type
	for items that can represent a player inventory, a chest,
	or any other type of container that can hold many stacks
	of items.

	They also can have special rules about what types of items
	they can contain, and in which slots. For example on the
	player quick slots, we can explicitly only allow primary
	weapons in slot 0 and 1, and secondary weapons in slot 2.
*/
UCLASS(NotBlueprintable)
class UFGItemContainer : public UObject//, public FTickableGameObject
{
	GENERATED_BODY()
public:

	//~ Begin Super
	virtual UWorld* GetWorld() const override
	{
		checkf(GetOuter(), TEXT("FG item container must have an outer."));
		return GetOuter()->GetWorld();
	}

	virtual void GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual void PostInitProperties() override;

#if UE_WITH_IRIS
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS
	//~ End Super

	const FFGItemStack* operator[](int32 Index)
	{
		return &ItemStacks[Index];
	}

	/*
		Returns which ELifetimeCondition should be used for this container
		to replicate to relevant connections. This can be used to change the
		condition if the property is set to use COND_Dynamic in an object's
		GetLifetimeReplicatedProps implementation.
	*/
	virtual ELifetimeCondition GetReplicationCondition() const;
	virtual bool ShouldReplicate() const;

	/*
		Find the first suitable item stack that we can either stack onto or
		an empty slot we can consume.

		returns INDEX_NONE if no suitable slot was found.
	*/
	virtual int32 AutoStack(const FFGItemStack Item);
	// @TODO: We need to account for the fact players can also
	// do move / split to different containers, therefore we
	// will need a net addressable target container.

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void OpenContainer(APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void CloseContainer(APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable)
	void MoveStack(int32 FromSlot, int32 ToSlot, UFGItemContainer* TargetContainer = nullptr);

	void TransferToSlot(int32 ToSlot, const FFGItemStack& Stack, int32 Amount = -1);

	/*
		Split an item stack, moving or combing a partial amount to a
		new slot.
	*/
	void SplitStack(int32 FromSlot, int32 ToSlot, int32 Amount, UFGItemContainer* TargetContainer = nullptr);

	UFUNCTION(BlueprintPure)
	bool IsValidSlot(int32 Slot) const;

	UFUNCTION(Client, Unreliable)
	void Client_SendDelta(const TArray<FFGItemStack>& Delta);

	UFUNCTION(BlueprintPure)
	int32 GetNumSlots() { return ItemStacks.Num(); }

	UFUNCTION(BlueprintPure)
	bool HasAuthority() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetItemStackElement(int32 Index, const FFGItemStack& Item);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 ConsumeItem(UFGItemStaticData* Item, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 ConsumeItemIndex(int32 Index, int32 Amount = 1);

	// Function we can override in the children to react when an index has been emptied or filled for the first time
	virtual void OnIndexPreChange(int32 Index) {};
	virtual void OnIndexPostChange(int32 Index) {};

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 GetItemCount(UFGItemStaticData* Item);

	const TArray<FFGItemStack>& GetItemStacks() const { return ItemStacks; }

	// Note, only use if you know what you are doing
	TArray<FFGItemStack>& GetItemStacks_Mutable();

protected:

	UPROPERTY(BlueprintReadOnly, Transient, Replicated)
	TArray<FFGItemStack> ItemStacks;

	UPROPERTY(Transient)
	TSet<APlayerState*> ViewingPlayers;
	
	/*
		Bind a filter lambda to this container which defines the
		rules for what items can be placed in which slots.
	*/
	template<typename FunctorType>
	inline void BindFilterLambda(FunctorType&& InFunctor)
	{
		FilterBinding = InFunctor;
	}

private:
	TFunction<void(UFGItemStaticData*, int32)> FilterBinding;
};
