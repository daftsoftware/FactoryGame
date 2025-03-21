// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "IFGInteractable.generated.h"

UINTERFACE(MinimalAPI)
class UFGInteractable : public UInterface
{
	GENERATED_BODY()
};

class IFGInteractable
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintNativeEvent)
	FText GetInteractText();
	
	UFUNCTION(BlueprintNativeEvent)
	bool CanInteractWith(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	void StartUse(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	void StopUse(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	void StartInteractPrimaryAction(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	void StopInteractPrimaryAction(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	void StartInteractSecondaryAction(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	void StopInteractSecondaryAction(APawn* Interactor);
};
