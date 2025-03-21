// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGActivatableWidget.h"
#include "FGModalWidget.generated.h"

/*
	Base class that should be inherited for modals.
	
	Exists for the sole reason of disallowing incorrect setup of
	activatable widget defaults for modals which are error-prone
	and easy to forget.
*/
UCLASS()
class FGUSERINTERFACE_API UFGModalWidget : public UFGActivatableWidget
{
	GENERATED_BODY()
public:
	
	UFGModalWidget();
};
