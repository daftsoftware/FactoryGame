// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Modules/ModuleInterface.h"

class FFGSteamDeckModule final : public IModuleInterface
{
public:

	static FFGSteamDeckModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FFGSteamDeckModule>("FGSteamDeck");
	}
	
	//~ Begin Super
	void StartupModule() override;
	//~ End Super

	/**
	 * Check if the game is running on Steam Deck.
	 * @return true if running on Steam Deck
	 */
	bool IsRunningOnSteamDeck() const;
	
private:
	
	/**
	 * Query Steam Deck status on startup, for loading correct device
	 * specific configuration files and device profiles.
	 * @return true if running on Steam Deck
	 */
	bool IsSteamDeck() const;

	/**
	 * Query if Steam Deck is running the game in desktop mode.
	 * @return true if SteamDeck is running the game in desktop mode
	 */
	bool IsInDesktopMode() const;

	/**
	 * Query if Steam Deck is running the game in desktop mode.
	 * @return true if SteamDeck is running the game in desktop mode
	 */
	bool IsInBigPictureMode() const;
};
