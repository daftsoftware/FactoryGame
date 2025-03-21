// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "CommonUserWidget.h"
#include "FGContainerSlotWidget.generated.h"

struct FFGItemStack;
class UFGItemContainer;
class UCommonLazyImage;
class UCommonTextBlock;

UCLASS()
class UFGContainerSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()
public:

	//~ Begin Super
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End Super

	// Caution: This binds our container UI to the container memory - use with care.
	UFUNCTION(BlueprintCallable)
	void BindSlot(UFGItemContainer* InContainer, int32 InSlotIndex);

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UFGItemContainer> OwningContainer;
	
	UPROPERTY(Meta = (BindWidget))
	UCommonLazyImage* StackIcon;
	
	UPROPERTY(Meta = (BindWidget))
	UCommonTextBlock* StackCountText;

	UPROPERTY(BlueprintReadOnly)
	int32 TrackedSlotIndex;
	
private:

	const FFGItemStack* OwningItemStack;
};
