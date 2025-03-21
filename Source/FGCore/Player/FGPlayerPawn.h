// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Character/FGAbilityPawn.h"
#include "GameplayTagContainer.h"
#include "Inventory/FGItemBackpack.h"
#include "GameplayAbilitySpecHandle.h"
#include "InputActionValue.h"
#include "FGPlayerPawn.generated.h"

class UDynamicMeshComponent;
class UFGVoxelPawnCollision;
class UFGWearableStaticData;
class USkeletalMeshComponent;
class UPrimitiveComponent;
struct FFGItemStack;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeQuickSlot, int32, Index);

UCLASS()
class FGCORE_API AFGPlayerPawn : public AFGAbilityPawn
{
	GENERATED_BODY()
public:
	
	AFGPlayerPawn();

	//~Begin Super
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void PawnClientRestart() override;
	void Tick(float DeltaSeconds) override;
#if UE_WITH_IRIS
	virtual void BeginReplication() override;
#endif
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	//~End Super
	
	void		SetHighlight(UObject* Target, bool HighlightEnabled = true, bool HighlightOwningActor = true);
	UObject*	FindInteractable(UObject* InObject);
	void		ResetInteractionTarget();
	void		Interact();
	void		StopInteract();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);

	/** Is this a predicting client, this is false in single player */
	bool IsPredictingClient() const;

	// Currently targeted comp by the player for interaction.
	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> TargetedComponent;

	// The interacted with item - while held.
	UPROPERTY()
	TWeakObjectPtr<UObject> CurrentlyInteracting;

	UPROPERTY(BlueprintReadOnly)
	FText InteractText;

	UFUNCTION(BlueprintCallable)
	void LocalHoldStack(UFGItemContainer* FromContainer, int32 FromSlot, bool bSplit = false);

	UFUNCTION(Client, Reliable)
	void ClientSetHeldStack(const FFGItemStack& Stack);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerTryHoldStack(UFGItemContainer* FromContainer, int32 FromSlot, bool bSplit = false);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerPlaceStack(int32 ToSlot, UFGItemContainer* ToContainer, int32 Amount = -1);

	UPROPERTY(BlueprintAssignable)
	FOnChangeQuickSlot OnChangeQuickSlot;
	
	UFGItemBackpack& GetBackpack() { return *Backpack; }
	const FFGItemStack* GetItemStack(int32 Index) { return (*Backpack)[Index]; }
	
	void CycleQuickItems(const FInputActionValue& Value);
	void DropCurrentItem(bool bInitiateThrow = false);

	UFUNCTION(Server, Reliable)
	void QuickSwapToItem(int32 Index);

	UFUNCTION()
	void OnRep_QuickItemIndex();
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_QuickItemIndex)
	int32 QuickItemIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> HeldItemTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UDynamicMeshComponent> HeldItemMesh;

	void SetHeldItemMesh(int32 ItemStackIndex);
	
	// Backpack is a players generic item space for pickups.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UFGItemBackpack> Backpack;
	
	// Slot for holdables
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UFGPlayerContainer> TransientSlot;

	UPROPERTY()
	FGameplayTagContainer ActiveModes;
};
