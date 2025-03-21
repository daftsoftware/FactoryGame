// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "CommonUserWidget.h"
#include "FGContainerWidget.generated.h"

class UFGItemContainer;

UCLASS()
class FGCORE_API UFGContainerWidget : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UFGItemContainer> Container;
};
