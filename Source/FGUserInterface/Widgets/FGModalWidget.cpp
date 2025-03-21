// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGModalWidget.h"

UFGModalWidget::UFGModalWidget()
{
	bAutoActivate = false;
	bIsModal = true;
	SetIsFocusable(false);
	bIsBackHandler = true;
	InputConfig = EFGWidgetInputMode::Menu;
}


