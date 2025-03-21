// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "EnhancedPlayerInput.h"
#include "FGPlayerInput.generated.h"

UCLASS(MinimalAPI)
class UFGPlayerInput : public UEnhancedPlayerInput
{
	GENERATED_BODY()
public:

	//~ Begin Super
	void ProcessInputStack(const TArray<UInputComponent*>& InputComponentStack, const float DeltaTime, const bool bGamePaused) override;
	//~ End Super

	/** Exec function to change the gamepad sensitivity */
	FGCORE_API void SetGamepadSensitivity(const float SensitivityX, const float SensitivityY);
	
	/** Sets both X and Y axis sensitivity on gamepad to the supplied value. */
	UFUNCTION(exec)
	void SetGamepadSensitivity(const float Sensitivity) { SetGamepadSensitivity(Sensitivity, Sensitivity); }
	
	/** Exec function to change the mouse sensitivity */
	FGCORE_API void SetMouseSensitivity(const float SensitivityX, const float SensitivityY);

	/** Sets both X and Y axis sensitivity on mouse to the supplied value. */
	void SetMouseSensitivity(const float Sensitivity) { SetMouseSensitivity(Sensitivity, Sensitivity); }
};
