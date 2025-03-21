// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Widgets/FGActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "UI/Game/FGHUDGame.h"
#include "FGGameContentWidget.generated.h"

class AFGHUD;
class UFGChatWidget;

UCLASS(Abstract)
class FGCORE_API UFGGameContentWidget final : public UFGActivatableWidget
{
	GENERATED_BODY()
public:
	AFGHUDGame* GetGameHUD();

	void OpenChatbox();
	void OpenChatboxCmd();
	
	// Route a ULocalMessage to the appropriate place - aka chat.
	void ReceiveMessage(APlayerState* Author, const FGameplayTag& MessageType, FText MessageText);

	// Chatbox for chat messages or other player facing text info.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFGChatWidget> Chatbox;
};
