// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGCoreMacros.h"
#include "Player/FGPlayerPawn.h"
#include "Player/FGPlayerController.h"
#include "FGItemSubsystem.h"

#include "HAL/IConsoleManager.h"

namespace FG
{

	///////////////////////////////////////////////////
	//~ Start Cmd FG.DumpBackpack

	static FAutoConsoleCommandWithWorld CmdDumpBackpack(
		TEXT("FG.DumpBackpack"),
		TEXT("Dumps all backpack items to the log."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			AFGPlayerController* PC =	World->GetFirstPlayerController<AFGPlayerController>();
			AFGPlayerPawn* Pawn = PC->GetPawn<AFGPlayerPawn>();

#if 0
			for(int32 Idx = 0; Idx < Pawn->Backpack.Num(); Idx++)
			{
				const FString& ItemName = Pawn->Backpack[Idx].ItemType->DisplayName.ToString();
				UE_LOG(LogFG, Display, TEXT("%s x%i"), *ItemName, Pawn->Backpack[Idx].Num());
			}
#endif
		})
	);
	
	//~ End Cmd FG.DumpBackpack
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	//~ Start Cmd FG.ListItems
	
	static FAutoConsoleCommandWithWorld CmdListItems(
		TEXT("FG.ListItems"),
		TEXT("Lists all items in the game."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if(auto* ItemSys = World->GetSubsystem<UFGItemSubsystem>())
			{
				for(FName ItemName : ItemSys->ListItemNames())
				{
					UE_LOG(LogFG, Display, TEXT("%s"), *ItemName.ToString());
				}
			}
		})
	);
	
	//~ End Cmd FG.ListItems
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	//~ Start Cmd FG.Give

	// Usage "FG.Give Grass 1"
	static FAutoConsoleCommandWithWorldAndArgs CmdGive(
		TEXT("FG.Give"),
		TEXT("Gives the player X of Item. Name | Count"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if(Args.Num() < 2)
			{
				UE_LOG(LogFG, Error, TEXT("FG.Give requires 2 arguments. ID | Count"));
				return;
			}

			const FName ItemID = *Args[0];
			const int32 Count = FCString::Atoi(*Args[1]);

			if(Count <= 0)
			{
				UE_LOG(LogFG, Error, TEXT("FG.Give requires a count greater than 0."));
				return;
			}

			AFGPlayerController* PC = World->GetFirstPlayerController<AFGPlayerController>();
			PC->Cheat(FString::Format(TEXT("GiveItem {0} {1}"), { *ItemID.ToString(), Count}));
		})
	);
	
	//~ End Cmd FG.Give
	///////////////////////////////////////////////////
}
