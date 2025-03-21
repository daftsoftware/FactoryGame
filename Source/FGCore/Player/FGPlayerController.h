// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/PlayerController.h"
#include "FGPlayerController.generated.h"

class UFGVoxelNetManager;
class UFGNetClock;
class UFGAbilitySystemComponent;
class AFGPlayerPawn;

/*
	FGPlayerController, does what it says on the tin mate.
*/
UCLASS()
class FGCORE_API AFGPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	AFGPlayerController();

	UFUNCTION(Exec)
	virtual void Say(const FString& Message);

	UFUNCTION(Reliable, Client)
	void ClientSay(APlayerState* MessageAuthor, const FString& Message, const FGameplayTag& MessageType);

	// This ServerRPC is the one that clients use to whisper and broadcasts messages, note that a different server-only function should be used for other type of relevant messages
	UFUNCTION(Reliable, Server)
	void ServerSay(const FString& Message);

	// The message is candidate of a whisper message
	void Whisper(const FString& Message);

	// Run a cheat command, if applicable, will RPC it through ServerCheat
	void Cheat(const FString& Msg);

	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheat(const FString& Msg);

	// Run a cheat command for all players, if applicable, will RPC it through ServerCheat
	void CheatAll(const FString& Msg);

	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheatAll(const FString& Msg);

	//~APlayerController interface
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	UFUNCTION(BlueprintCallable)
	AFGPlayerPawn* GetFGPlayerPawn() const;

	UFUNCTION(BlueprintCallable)
	UFGAbilitySystemComponent* GetFGAbilitySystemComponent() const;

private:

	UPROPERTY()
	TObjectPtr<UFGNetClock> NetClock;

	UPROPERTY()
	TObjectPtr<UFGVoxelNetManager> VoxelNetManager;
};
