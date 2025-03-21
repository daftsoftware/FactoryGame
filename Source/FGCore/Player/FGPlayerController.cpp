// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPlayerController.h"
#include "FGPlayerState.h"
#include "..\System\FGGameData.h"
#include "System/FGLocalMessage.h"
#include "System/FGNetClock.h"
#include "Debug/FGCheatManager.h"
#include "FGGameplayTags.h"
#include "FGCoreMacros.h"
#include "EngineUtils.h"
#include "FGPlayerPawn.h"
#include "AbilitySystem/FGAbilitySystemComponent.h"

#include "GameFramework/LocalMessage.h"
#include "Online/FGVoxelNetManager.h"

AFGPlayerController::AFGPlayerController()
{
	CheatClass = UFGCheatManager::StaticClass();
	
	NetClock = CreateDefaultSubobject<UFGNetClock>(TEXT("NetClock"));
	VoxelNetManager = CreateDefaultSubobject<UFGVoxelNetManager>(TEXT("VoxelNetManager"));
}

void AFGPlayerController::Say(const FString& Message)
{
	ServerSay(Message.Left(UFGGameData::Get().MaxChatMessageLength));
}

void AFGPlayerController::Whisper(const FString& Message)
{
	// Figure out who we are talking too...
	if (!HasAuthority())
	{
		return;
	}

	FString TargetPlayerName;
	FString FinalMessage = Message;

	bool bSent = false;

	// Look for a local player controller to send to...
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AFGPlayerController* PlayerController = Cast<AFGPlayerController>(*Iterator);
		if (PlayerController != nullptr && PlayerController->PlayerState != nullptr)
		{
			APlayerState* TargetPlayerState = PlayerController->PlayerState;
			if (TargetPlayerState != nullptr)
			{
				TargetPlayerName = TargetPlayerState->GetPlayerName();

				if (Message.Left(TargetPlayerName.Len()).Equals(TargetPlayerName, ESearchCase::IgnoreCase))
				{
					FinalMessage = FinalMessage.Right(FinalMessage.Len() - TargetPlayerName.Len()).TrimStartAndEnd();
					bSent = true;

					TSharedPtr<const FUniqueNetId> Id = PlayerController->PlayerState->GetUniqueId().GetUniqueNetId();
					bool bIsMuted = Id.IsValid() && IsPlayerMuted(Id.ToSharedRef().Get());

					if (!bIsMuted)
					{
						PlayerController->ClientSay(PlayerState, FinalMessage, TAG_CHAT_CHANNEL_WHISPER);
					}
					FinalMessage = FString::Printf(TEXT("to %s \"%s\""), *TargetPlayerName, *FinalMessage);
					break;
				}
			}
		}
	}

	if (bSent)
	{
		// Make sure I see that I sent it..
		ClientSay(PlayerState, FinalMessage, TAG_CHAT_CHANNEL_WHISPER);
	}
}

void AFGPlayerController::ClientSay_Implementation(APlayerState* MessageAuthor, const FString& Message, const FGameplayTag& MessageType)
{
	FClientReceiveData ClientData;
	ClientData.LocalPC = this;
	ClientData.MessageType = MessageType.GetTagName(); // GameplayTag Chat Destination
	ClientData.RelatedPlayerState_1 = MessageAuthor;
	ClientData.MessageString = Message;

	UFGChatMessage::StaticClass()->GetDefaultObject<UFGChatMessage>()->ClientReceiveChat(ClientData, ClientData.MessageType);
}

void AFGPlayerController::ServerSay_Implementation(const FString& Message)
{
	if(PlayerState == nullptr) // No PlayerState, just drop our message.
	{
		return;
	}

	// Apply local formatting like trimming spaces. Trimming it again to MaxLength 
	// in case a player bypassed the local trimming.
	FString TrimmedMessage = Message.Left(UFGGameData::Get().MaxChatMessageLength);
	TrimmedMessage = TrimmedMessage.TrimStartAndEnd();

	if (TrimmedMessage.Left(2) == TEXT("/w"))
	{
		// Remove the "/w "
		TrimmedMessage = TrimmedMessage.Right(TrimmedMessage.Len() -3);
		Whisper(TrimmedMessage);
		return;
	}

	// @TODO: Sanitize the message!!!

	bool bSpectatorMsg = PlayerState->IsOnlyASpectator();

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (AFGPlayerController* PlayerController = Cast<AFGPlayerController>(*Iterator))
		{
			TSharedPtr<const FUniqueNetId> Id = PlayerState->GetUniqueId().GetUniqueNetId();
			bool bIsMuted = Id.IsValid() && PlayerController->IsPlayerMuted(Id.ToSharedRef().Get());

			// Dont send spectator chat to players
			if (PlayerController->PlayerState != nullptr && (!bSpectatorMsg || PlayerController->PlayerState->IsOnlyASpectator()) && !bIsMuted)
			{
				PlayerController->ClientSay(PlayerState, TrimmedMessage, TAG_CHAT_CHANNEL_ALL);
			}
		}
	}
}

void AFGPlayerController::Cheat(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (GetNetMode() == NM_Client)
	{
		// Automatically send cheat to server for convenience.
		ServerCheat(Msg);
		return;
	}
	ClientMessage(ConsoleCommand(Msg));
#endif // #if USING_CHEAT_MANAGER
}

void AFGPlayerController::CheatAll(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (GetNetMode() == NM_Client)
	{
		// Automatically send cheat to server for convenience.
		ServerCheatAll(Msg);
		return;
	}
	UE_LOG(LogFG, Warning, TEXT("ServerCheatAll: %s"), *Msg);
	for (TActorIterator<AFGPlayerController> It(GetWorld()); It; ++It)
	{
		AFGPlayerController* FGPC = (*It);
		if (FGPC)
		{
			FGPC->ClientMessage(FGPC->ConsoleCommand(Msg));
		}
	}
#endif // #if USING_CHEAT_MANAGER
}

void AFGPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UFGAbilitySystemComponent* FGASC = GetFGAbilitySystemComponent())
	{
		FGASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

AFGPlayerPawn* AFGPlayerController::GetFGPlayerPawn() const
{
	return CastChecked<AFGPlayerPawn>(GetPawn(), ECastCheckedType::NullAllowed);
}

UFGAbilitySystemComponent* AFGPlayerController::GetFGAbilitySystemComponent() const
{
	const AFGPlayerPawn* PlayerPawn = GetFGPlayerPawn();
	return (PlayerPawn ? PlayerPawn->GetFGAbilitySystemComponent() : nullptr);
}

void AFGPlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogFG, Warning, TEXT("ServerCheat: %s"), *Msg);
		ClientMessage(ConsoleCommand(Msg));
	}
#endif // #if USING_CHEAT_MANAGER
}

bool AFGPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void AFGPlayerController::ServerCheatAll_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	UE_LOG(LogFG, Warning, TEXT("ServerCheatAll: %s"), *Msg);
	for (TActorIterator<AFGPlayerController> It(GetWorld()); It; ++It)
	{
		AFGPlayerController* FGPC = (*It);
		if (FGPC)
		{
			FGPC->ClientMessage(FGPC->ConsoleCommand(Msg));
		}
	}
#endif // #if USING_CHEAT_MANAGER
}

bool AFGPlayerController::ServerCheatAll_Validate(const FString& Msg)
{
	return true;
}