// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPlayerInput.h"

#include "FGPlayerPawn.h"
#include "..\Debug\FGCheatManager.h"

void UFGPlayerInput::ProcessInputStack(const TArray<UInputComponent*>& InputComponentStack, const float DeltaTime,
                                        const bool bGamePaused)
{
	Super::ProcessInputStack(InputComponentStack, DeltaTime, bGamePaused);
	
#if 0
	// Directly forward delete key to pop the cheat menu.
	if (WasJustPressed(EKeys::Delete))
	{
		if(GWorld)
		{
			if(auto* PC = GWorld->GetFirstPlayerController())
			{
				CastChecked<UFGCheatManager>(PC->CheatManager)->ToggleCheatMenu();
			}
		}
	}
#endif

	int32 ChangedQuickSlotIndex = INDEX_NONE;

	if (WasJustPressed(EKeys::One))
	{
		ChangedQuickSlotIndex = 0;
	}
	else if (WasJustPressed(EKeys::Two))
	{
		ChangedQuickSlotIndex = 1;
	}
	else if (WasJustPressed(EKeys::Three))
	{
		ChangedQuickSlotIndex = 2;
	}
	else if (WasJustPressed(EKeys::Four))
	{
		ChangedQuickSlotIndex = 3;
	}
	else if (WasJustPressed(EKeys::Five))
	{
		ChangedQuickSlotIndex = 4;
	}
	else if (WasJustPressed(EKeys::Six))
	{
		ChangedQuickSlotIndex = 5;
	}
	else if (WasJustPressed(EKeys::Seven))
	{
		ChangedQuickSlotIndex = 6;
	}
	else if (WasJustPressed(EKeys::Eight))
	{
		ChangedQuickSlotIndex = 7;
	}
	else if (WasJustPressed(EKeys::Nine))
	{
		ChangedQuickSlotIndex = 8;
	}
	else if (WasJustPressed(EKeys::Zero))
	{
		ChangedQuickSlotIndex = 9;
	}

	if(ChangedQuickSlotIndex != INDEX_NONE)
	{
		if(GWorld)
		{
			if(APlayerController* PC = GWorld->GetFirstPlayerController())
			{
				if(AFGPlayerPawn* Pawn = PC->GetPawn<AFGPlayerPawn>())
				{
					Pawn->QuickSwapToItem(ChangedQuickSlotIndex);
				}
			}
		}
	}
}

void UFGPlayerInput::SetGamepadSensitivity(const float SensitivityX, const float SensitivityY)
{
	
}

// Base UPlayerInput version doesn't account for Mouse2D.
void UFGPlayerInput::SetMouseSensitivity(const float SensitivityX, const float SensitivityY)
{
	//FInputAxisProperties MouseAxisProps;
	//if (GetAxisProperties(EKeys::MouseX, MouseAxisProps))
	//{
	//	MouseAxisProps.Sensitivity = SensitivityX;
	//	SetAxisProperties(EKeys::MouseX, MouseAxisProps);
	//}
	//if (GetAxisProperties(EKeys::MouseY, MouseAxisProps))
	//{
	//	MouseAxisProps.Sensitivity = SensitivityY;
	//	SetAxisProperties(EKeys::MouseY, MouseAxisProps);
	//}
	//if (GetAxisProperties(EKeys::Mouse2D, MouseAxisProps))
	//{
	//	
	//}
}
