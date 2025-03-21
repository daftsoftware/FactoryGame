// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGGameContentWidget.h"
#include "UI/Game/Widgets/FGChatWidget.h"
#include "UI/Game/FGHUDGame.h"
#include "FGGameplayTags.h"

AFGHUDGame* UFGGameContentWidget::GetGameHUD()
{
	return GetOwningPlayer()->GetHUD<AFGHUDGame>();
}

void UFGGameContentWidget::OpenChatbox()
{
	if (!Chatbox->IsChatting())
	{
		Chatbox->ShowChat();
	}
	else
	{
		Chatbox->HideChat();
	}
}

void UFGGameContentWidget::OpenChatboxCmd()
{
	UE_LOG(LogTemp, Display, TEXT("Begin chat command"));
}

void UFGGameContentWidget::ReceiveMessage(APlayerState* Author, const FGameplayTag& MessageType, FText MessageText)
{
	if (MessageType.MatchesTag(TAG_CHAT_CHANNEL))
	{
		Chatbox->ReceiveMessage(Author, MessageType, MessageText);
	}
}
