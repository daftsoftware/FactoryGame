// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGContainerSlotWidget.h"
#include "Inventory/FGItemStack.h"
#include "Inventory/FGItemContainer.h"

#include "CommonLazyImage.h"
#include "CommonTextBlock.h"

void UFGContainerSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// @TODO: Bit of a hack, should hook to container update delegates.

	if(!OwningItemStack) // Slot not bound yet.
	{
		return;
	}
	
	// Readback and update state from inventory slot.

	if(OwningItemStack->StaticData) // Valid Item.
	{
		StackIcon->SetBrush(OwningItemStack->StaticData->Icon);
	}
	else // Empty slot.
	{
		static FSlateBrush EmptyBrush;
		EmptyBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
		
		StackIcon->SetBrush(EmptyBrush);
	}

	if(OwningItemStack->StackSize > 1) // Stacked Item.
	{
		StackCountText->SetText(FText::AsNumber(OwningItemStack->StackSize));
	}
	else // Single Item or Invalid.
	{
		StackCountText->SetText(FText::GetEmpty());
	}
}

void UFGContainerSlotWidget::BindSlot(UFGItemContainer* InContainer, int32 InSlotIndex)
{
	checkf(InContainer, TEXT("Invalid container."));
	checkf(InContainer->GetItemStacks().IsValidIndex(InSlotIndex), TEXT("Invalid slot index."));

	OwningContainer = InContainer;
	TrackedSlotIndex = InSlotIndex;
	OwningItemStack = &InContainer->GetItemStacks()[InSlotIndex];
}
