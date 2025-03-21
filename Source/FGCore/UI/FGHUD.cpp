// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGHUD.h"
#include "System/FGGameData.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/FGActivatableWidget.h"
#include "Widgets/FGModalWidget.h"
#include "Widgets/FGLayoutWidget.h"

void AFGHUD::BeginPlay()
{
	Super::BeginPlay();

	checkf(!GetLayoutClass().IsNull(), TEXT("Invalid Layout Class."));
	
    Layout = CreateWidget<UFGLayoutWidget>(PlayerOwner, GetLayoutClass().LoadSynchronous());
    checkf(Layout, TEXT("Layout failed to instantiate."));
	
    Layout->AddToViewport(0);
}

void AFGHUD::AddToContentLayer(TSoftClassPtr<UFGActivatableWidget> WidgetClass)
{
	checkf(Layout, TEXT("Layout is not set on HUD"));
	Layout->ContentLayer->AddWidget(WidgetClass.LoadSynchronous());
}

void AFGHUD::AddToMenuLayer(TSoftClassPtr<UFGActivatableWidget> WidgetClass)
{
	checkf(Layout, TEXT("Layout is not set on HUD"));
	Layout->MenuLayer->AddWidget(WidgetClass.LoadSynchronous());
}

void AFGHUD::AddToModalLayer(TSoftClassPtr<UFGModalWidget> WidgetClass)
{
	checkf(Layout, TEXT("Layout is not set on HUD"));
	Layout->ModalLayer->AddWidget(WidgetClass.LoadSynchronous());
}

