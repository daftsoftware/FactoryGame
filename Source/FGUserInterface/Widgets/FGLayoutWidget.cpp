// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGLayoutWidget.h"

#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"

UFGLayoutWidget::UFGLayoutWidget()
{
}

void UFGLayoutWidget::AddModal(TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	ModalLayer->AddWidget(WidgetClass.LoadSynchronous());
}

void UFGLayoutWidget::RemoveModal(UCommonActivatableWidget* Widget)
{
	ModalLayer->RemoveWidget(*Widget);
}
