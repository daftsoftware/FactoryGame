// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGChatWidget.h"
#include "Player/FGPlayerController.h"
#include "FGGameplayTags.h"

#include "GameFramework/PlayerState.h"
#include "Components/EditableText.h"
#include "Components/RichTextBlock.h"
#include "Components/Border.h"

void UFGChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(ChatInput)
	{
		ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatCommitted);
	}

	// Add allowed chat channels
	AllowedChannels.AddTag(TAG_CHAT_CHANNEL_ALL);
	AllowedChannels.AddTag(TAG_CHAT_CHANNEL_WHISPER);

	// Register colors for channels we don't want to be white.

	// User Channels
	RegisterChannelColor(TAG_CHAT_CHANNEL_WHISPER, FLinearColor(1.f, 0.f, 1.f, 1.f));

	// System Channels
	RegisterChannelColor(TAG_CHAT_CHANNEL_INFO, FLinearColor(1.f, 1.f, 0.f, 1.f));
	RegisterChannelColor(TAG_CHAT_CHANNEL_WARNING, FLinearColor(1.f, 0.f, 0.f, 1.f));

	UpdateVisibility(false, bIsChatting);
	ChatInput->SetVisibility(ESlateVisibility::Hidden);
	ChatInputBG->SetVisibility(ESlateVisibility::Hidden);
}

void UFGChatWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsChatting && ChatInput) // Chat open.
	{
		// Force focus every frame onto the chatbox input.
		ChatInput->SetKeyboardFocus();
	}
	else // Chat closed.
	{
		int32 LastVisibleEntries = VisibleEntries;
		VisibleEntries = 0;
		
		// Calculate how many entries to show.
		for(int32 Entry = 0; Entry < ChatHistory.Num(); Entry++)
		{
			// Check if entry is older than 2 seconds.
			if(FApp::GetGameTime() - ChatHistory[Entry].TimeReceived < ClosedVisibilityTime)
			{
				VisibleEntries++;
			}
		}
		
		// Refresh chat.
		if(LastVisibleEntries != VisibleEntries)
		{
			RebuildChat();
		}
		
		// Exit UI mode
		// TODO: For a reason this makes the mouse jitter, study alternatives.
		// FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
	ChatHistoryBG->SetVisibility(VisibleEntries > 0 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	
	// TODO: For a reason this makes the mouse jitter, study alternatives.
	// FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetType(EMouseCursor::None);
}

FReply UFGChatWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	return FReply::Handled().ReleaseMouseCapture().LockMouseToWidget(ChatInput->TakeWidget());
}

void UFGChatWidget::ReceiveMessage(APlayerState* Author, const FGameplayTag& MessageType, FText MessageText)
{
	// @TODO: Maybe some nice fancy filtering with tags?
	
	if(Author)
	{
		AddChatLine(
			MessageType,
			FText::FromString(FString::Printf(
				TEXT("<%s> %s"), 
				*Author->GetPlayerName(), 
				*MessageText.ToString())
			)
		);
	}
}

void UFGChatWidget::ShowChat()
{
	bIsChatting = true;
	UpdateVisibility(true, bIsChatting);
	ChatInput->SetVisibility(ESlateVisibility::Visible);
	ChatInputBG->SetVisibility(ESlateVisibility::Visible);
	ChatInput->SetText(FText());
	ChatHistoryBG->SetVisibility(ESlateVisibility::Visible);
	
	VisibleEntries = FMath::Min(ChatHistory.Num(), MaxVisibleEntries);
	RebuildChat();
}

void UFGChatWidget::HideChat()
{
	bIsChatting = false;
	UpdateVisibility(true, bIsChatting);
	ChatInput->SetVisibility(ESlateVisibility::Hidden);
	ChatInputBG->SetVisibility(ESlateVisibility::Hidden);
	
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
	FSlateApplication::Get().ClearKeyboardFocus();
}

void UFGChatWidget::OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (!ChatInput->GetText().IsEmpty())
		{
			// Send the message on the chosen channel
			SendChatMessage(ChatInput->GetText().ToString());

			// Clear the text
			ChatInput->SetText(FText());

			OnChatMessageSent();

			HideChat();
		}
	}
}

void UFGChatWidget::SendChatMessage(const FString& Message)
{
	if (AFGPlayerController* Controller = GetOwningPlayer<AFGPlayerController>())
	{
		Controller->Say(Message);
	}
}

FString UFGChatWidget::GetChannelDisplayName(FGameplayTag Channel)
{
	// Find end segment of tag aka "All" from Chat.Channel.All
	FString Tag = Channel.GetTagName().ToString();
	int32 Substring = Tag.Find(
		TEXT("."),
		ESearchCase::IgnoreCase,
		ESearchDir::FromEnd
	);

	// Chop tag at the "." delimeter, discarding it (+1) and everything to the left.
	return Tag.RightChop(Substring + 1);
}

FSlateColor UFGChatWidget::GetChannelColor(FGameplayTag Channel)
{
	// If we registered a color, use that.
	if(ChannelColors.Contains(Channel))
	{
		return ChannelColors[Channel];
	}
	return FColor(1.f, 1.f, 1.f, 1.f); // Otherwise, default to white.
}

void UFGChatWidget::RegisterChannelColor(FGameplayTag Channel, FSlateColor Color)
{
	ChannelColors.Add(Channel, Color);
}

void UFGChatWidget::RebuildChat()
{
	FString Rebuild = TEXT("");
	for(int32 Entry = ChatHistory.Num() - VisibleEntries; Entry < ChatHistory.Num(); Entry++)
	{
		Rebuild += Rebuild.IsEmpty() ?
			*ChatHistory[Entry].ChatString :
			FString::Printf(TEXT("\n%s"), *ChatHistory[Entry].ChatString);
	}
	ChatHistoryView->SetText(FText::FromString(*Rebuild));
}

void UFGChatWidget::AddChatLine(FGameplayTag Channel, const FText &ChatString)
{
	if(ChatHistoryView)
	{
		// @TODO: Move clientside chat formatting here.
		
		ChatHistory.Add(FFGChatEntry(
			FApp::GetGameTime(),
			nullptr, // @FIXME: Author playerstate.
			Channel,
			ChatString.ToString()
		));

		FString Rebuild = ChatHistoryView->GetText().ToString(); // Printf is buggy asf with ternary so gg.
		Rebuild += ChatHistoryView->GetText().IsEmpty() ?
			*ChatString.ToString() :
			FString::Printf(TEXT("\n%s"), *ChatString.ToString());
		
		// Append to the chat history view.
		ChatHistoryView->SetText(FText::FromString(*Rebuild));
	}
}
