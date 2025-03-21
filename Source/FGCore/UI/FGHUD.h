// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/HUD.h"
#include "FGHUD.generated.h"

class UFGActivatableWidget;
class UFGModalWidget;
class UFGLayoutWidget;

/*
	Generic glue class for any and all UI.
*/
UCLASS()
class FGCORE_API AFGHUD : public AHUD
{
	GENERATED_BODY()
public:

	//~ Begin Super
	virtual void BeginPlay() override;
	//~ End Super

	virtual TSoftClassPtr<UFGLayoutWidget> GetLayoutClass() const { return TSoftClassPtr<UFGLayoutWidget>(); }

	virtual void AddToContentLayer(TSoftClassPtr<UFGActivatableWidget> WidgetClass);
	virtual void AddToMenuLayer(TSoftClassPtr<UFGActivatableWidget> WidgetClass);
	virtual void AddToModalLayer(TSoftClassPtr<UFGModalWidget> WidgetClass);

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    UFGLayoutWidget* Layout;
};
