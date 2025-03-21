// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "FGChatWidget.generated.h"

class URichTextBlock;
class UTextBlock;
class UEditableText;
class UBorder;

USTRUCT(BlueprintType)
struct FFGChatChannelData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSlateBrush ChatIcon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSlateColor ChatColor;
};

UENUM()
enum class EFGChatState : uint8
{
	Active,		// User is actively typing in the chat.
	Dormant,	// A message was recently recieved in the chat.
	Hidden		// No messages for a while, chat fully hidden.
};

USTRUCT(BlueprintType)
struct FFGChatEntry
{
	GENERATED_BODY()

	// Time Received.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float TimeReceived;

	// Author of this chat message.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	APlayerState* Author;
	
	// Channel type for this chat message.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FGameplayTag ChatChannel;

	// Source string of this chat message.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString ChatString;

	FFGChatEntry() = default;
	FFGChatEntry(float InTimeRec, APlayerState* InAuthor, const FGameplayTag& InChatChannel, const FString& InChatString)
		: TimeReceived(InTimeRec), Author(InAuthor), ChatChannel(InChatChannel), ChatString(InChatString) {}
};

/*
	UFGUIChat.
	Serves as the main chat widget for FGGame.
*/
UCLASS()
class FGCORE_API UFGChatWidget : public UCommonUserWidget
{
	GENERATED_BODY()
public:

	void NativeConstruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	void ReceiveMessage(APlayerState* Author, const FGameplayTag& MessageType, FText MessageText);

	void ShowChat();
	void HideChat();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateVisibility(bool Animate, bool NewVisibility);

	UFUNCTION(BlueprintPure)
	FString GetChannelDisplayName(FGameplayTag Channel);
	
	void SendChatMessage(const FString& Message);
	
	FSlateColor GetChannelColor(FGameplayTag Channel);
	void RegisterChannelColor(FGameplayTag Channel, FSlateColor Color);

	bool IsChatting() { return bIsChatting; }
	void RebuildChat();

	int32 VisibleEntries = 0;
	int32 MaxVisibleEntries = 10;
	float ClosedVisibilityTime = 2.f;

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void OnChatChannelChanged();

	UFUNCTION(BlueprintImplementableEvent)
	void OnChatMessageSent();

	UFUNCTION(BlueprintCallable)
	void OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void AddChatLine(FGameplayTag Channel, const FText &ChatString);

private:

	FGameplayTagContainer 				AllowedChannels;
	TMap<FGameplayTag, FSlateColor>		ChannelColors;

	/** The array of chat history. */
	UPROPERTY()
	TArray<FFGChatEntry> ChatHistory;

	bool bIsChatting = false;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UBorder* ChatHistoryBG;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	URichTextBlock* ChatHistoryView;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UBorder* ChatInputBG;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UEditableText* ChatInput;
};
