// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Components/Widget.h"
#include "UObject/Object.h"
#include "FGUserInterfaceStatics.generated.h"

class AFGHUD;
class UFGActivatableWidget;
class UFGModalWidget;

UCLASS()
class UFGUserInterfaceStatics : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Meta = (HidePin = "WorldContext", DefaultToSelf ="WorldContext"))
	static AFGHUD* GetActiveHUD(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Meta = (HidePin = "WorldContext", DefaultToSelf ="WorldContext"))
	static void AddWidgetToContentLayer(UObject* WorldContext, TSoftClassPtr<UFGActivatableWidget> Widget);

	UFUNCTION(BlueprintCallable, Meta = (HidePin = "WorldContext", DefaultToSelf ="WorldContext"))
	static void AddWidgetToMenuLayer(UObject* WorldContext, TSoftClassPtr<UFGActivatableWidget> Widget);

	UFUNCTION(BlueprintCallable, Meta = (HidePin = "WorldContext", DefaultToSelf ="WorldContext"))
	static void AddWidgetToModalLayer(UObject* WorldContext, TSoftClassPtr<UFGModalWidget> Widget);

	UFUNCTION(BlueprintPure)
	static UWidget* GetFocusedWidget();
};
