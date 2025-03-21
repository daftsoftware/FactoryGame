// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "CommonUserWidget.h"
#include "FGLayoutWidget.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetQueue;
class UCommonActivatableWidgetStack;
class UOverlay;

UCLASS(Abstract)
class FGUSERINTERFACE_API UFGLayoutWidget final : public UCommonUserWidget
{
	GENERATED_BODY()
public:

	UFGLayoutWidget();
	
	// UI Layer Helpers.
	void AddModal(TSoftClassPtr<UCommonActivatableWidget> WidgetClass);
	void RemoveModal(UCommonActivatableWidget* Widget);

	UPROPERTY(Meta=(BindWidget))
    UOverlay* Layers;
    
    // BEGIN LAYERS

	// Content layer for persistent Game UI (Game Input).
    UPROPERTY(BlueprintReadOnly, Meta=(BindWidget))
    UCommonActivatableWidgetStack* ContentLayer;

	// Content layer for Menu UI (Menu Input).
    UPROPERTY(BlueprintReadOnly, Meta=(BindWidget))
    UCommonActivatableWidgetStack* MenuLayer;
	
	// Content layer for popups and other modal UI (Menu Input).
    UPROPERTY(BlueprintReadOnly, Meta=(BindWidget))
    UCommonActivatableWidgetQueue* ModalLayer;

	// END LAYERS
};