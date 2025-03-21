// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGContainerActionWidget.h"
#include "Inventory/FGItemStack.h"
#include "Inventory/FGItemContainer.h"

#include "CommonTextBlock.h"
#include "CommonLazyImage.h"

TWeakObjectPtr<UFGContainerActionWidget> UFGContainerActionWidget::ActiveAction;

// We don't use native tick because this elem must tick when hidden.
void UFGContainerActionWidget::Tick(float DeltaTime)
{
	if(!OwningItemStack) // Slot not bound yet.
	{
		// @TODO: Destroy action.
		return;
	}

	// Update op visual position to mouse.
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if(PC)
	{
		FVector2D MousePos;
		if(PC->GetMousePosition(MousePos.X, MousePos.Y))
		{
			// @FIXME: This is viewport scale, so we need the widget scale / 2
			SetPositionInViewport(MousePos - FVector2D(64.0));
			SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
	// @TODO: Bit of a hack, should hook to container update delegates.
	
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

	CurrentStackSize = OwningItemStack->StackSize;
	
	if(CurrentStackSize > 1) // Stacked Item.
	{
		StackCountText->SetText(FText::AsNumber(CurrentStackSize));
	}
	else // Single Item or Invalid.
	{
		StackCountText->SetText(FText::GetEmpty());
	}
}

void UFGContainerActionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	checkf(!ActiveAction.IsValid(), TEXT("Only one container action can be active at a time!"));
	ActiveAction = this;
}

void UFGContainerActionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	ActiveAction.Reset();
}

void UFGContainerActionWidget::BindSlot(UFGItemContainer* InContainer, int32 InSlotIndex)
{
	checkf(InContainer, TEXT("Invalid container."));
	checkf(InContainer->GetItemStacks().IsValidIndex(InSlotIndex), TEXT("Invalid slot index."));

	FromContainer = InContainer;
	FromSlotIndex = InSlotIndex;
	OwningItemStack = &InContainer->GetItemStacks()[InSlotIndex];
}

UFGContainerActionWidget* UFGContainerActionWidget::GetCurrentContainerAction()
{
	return ActiveAction.Get();
}

void UFGContainerActionWidget::Cancel()
{
	RemoveFromParent();
}
