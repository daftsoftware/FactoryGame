// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGLocalMessage.h"
#include "UI/Game/FGHUDGame.h"

void UFGChatMessage::ClientReceiveChat(const FClientReceiveData& ClientData, FName Destination) const
{
	checkf(ClientData.LocalPC, TEXT("Invalid LocalPC in FClientReceiveData payload."));

	if(AFGHUDGame* HUD = ClientData.LocalPC->GetHUD<AFGHUDGame>())
	{
		FText LocalMessageText = FText::FromString(*ClientData.MessageString);

		// Forward message to the HUD.
		HUD->ReceiveLocalMessage(
			GetClass(), 
			ClientData.RelatedPlayerState_1, 
			ClientData.RelatedPlayerState_2,
			ClientData.MessageType,
			LocalMessageText,
			ClientData.OptionalObject
		);
	}
}