// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "CommonUserWidget.h"
#include "FGContainerActionWidget.generated.h"

struct FFGItemStack;
class UFGItemContainer;
class UCommonLazyImage;
class UCommonTextBlock;

UENUM(BlueprintType)
enum class EFGContainerOp : uint8
{
	Move,
	Split,
};

/*
	Container action singleton widget.
	There should only be one of these at a time, since a player
	can only be performing actions on one container at a time.
*/
UCLASS()
class UFGContainerActionWidget : public UCommonUserWidget, public FTickableGameObject
{
	GENERATED_BODY()
public:

	//~ Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFGUIContainerAction, STATGROUP_Tickables) }
	//~ End FTickableGameObject
	
	//~ Begin Super
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~ End Super
	
	// Caution: This binds our container UI to the container memory - use with care.
	UFUNCTION(BlueprintCallable)
	void BindSlot(UFGItemContainer* InContainer, int32 InSlotIndex);

	UFUNCTION(BlueprintPure)
	static UFGContainerActionWidget* GetCurrentContainerAction();

	UFUNCTION(BlueprintCallable)
	void Cancel();
	
	UPROPERTY(BlueprintReadWrite, Meta=(ExposeOnSpawn))
	EFGContainerOp Operation;
	
	UPROPERTY(Meta = (BindWidget))
	UCommonLazyImage* StackIcon;
	
	UPROPERTY(Meta = (BindWidget))
	UCommonTextBlock* StackCountText;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UFGItemContainer> FromContainer;
	
	UPROPERTY(BlueprintReadOnly)
	int32 FromSlotIndex;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentStackSize = INDEX_NONE;
	
	UPROPERTY(BlueprintReadWrite)
	bool SmartSplitting = false;

	UPROPERTY(BlueprintReadWrite)
	TSet<int32> TrackedSmartSplits;
	
private:

	const FFGItemStack* OwningItemStack;
	static TWeakObjectPtr<ThisClass> ActiveAction;
};
