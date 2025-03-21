// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "UObject/Object.h"
#include "FGSteamDeckStatics.generated.h"

UCLASS()
class UFGSteamDeckStatics : public UObject
{
	GENERATED_BODY()
public:

	/**
     * Check if the game is running on Steam Deck.
     * @return true if running on Steam Deck
     */
	UFUNCTION(BlueprintPure, Category = "Steam Deck")
	static bool IsRunningOnSteamDeck();
};