// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGNetClock.h"

#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"

namespace FG
{
	static bool DebugNetClockTimings = false;
	FAutoConsoleVariableRef CVarDebugNetClockTimings (
		TEXT("FG.NetClock.DebugTimings"),
		DebugNetClockTimings,
		TEXT("Enables timing debug for network clock."),
		ECVF_Default
	);
}

////////////////////////////////////////////////////////
//~ Begin NetClock - Thx Vori <3

UFGNetClock::UFGNetClock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bHighPriority = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
}

UFGNetClock* UFGNetClock::Get(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	if(!World)
	{
		return nullptr;
	}
	
	if(APlayerController* PC = World->GetFirstPlayerController())
	{
		return PC->FindComponentByClass<UFGNetClock>();
	}
	return nullptr;
}

void UFGNetClock::BeginPlay()
{
	Super::BeginPlay();

	if(GetWorld()->GetNetMode() != ENetMode::NM_Client) // NET - SERVER ONLY
	{
		// Does this somehow rerun on server on clients initial rep? wtf.
		ServerEpoch = FApp::GetCurrentTime();
		ReadyForUse = true;

		// Post login players and sync their epoch with the server's
		FGameModeEvents::GameModePostLoginEvent.AddWeakLambda(this, [this](
			AGameModeBase* GameMode, APlayerController* NewPlayer)
		{
			ThisClass* ClientClock = NewPlayer->FindComponentByClass<ThisClass>();
			ClientClock->ClientSetEpoch(ServerEpoch);
		});
	}
}

void UFGNetClock::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!IsReadyForUse()) // Early out, no epoch.
	{
		return;
	}

	// Calculate new network time adjusted for drift.
	LastNetworkTime = CurrentNetworkTime;
	CurrentNetworkTime = GetNetAbsoluteTime() + CurrentDrift;

	// Clients request a resync per the update frequency.
	if(GetWorld()->GetNetMode() == ENetMode::NM_Client) // NET - CLIENT ONLY
	{
		LastNetworkClockUpdate += FApp::GetDeltaTime();
		if(LastNetworkClockUpdate > NetworkClockUpdateFrequency)
		{
			LastNetworkClockUpdate -= NetworkClockUpdateFrequency;
			
			// Send T0 to Server.
			ServerRequestTime(GetNetAbsoluteTime());

			// Temp
			LastT0 = GetNetAbsoluteTime();
		}
	}

	if(FG::DebugNetClockTimings)
	{
		// stub
	}
}

bool UFGNetClock::IsReadyForUse() const
{
	return ReadyForUse;
}

double UFGNetClock::GetNetDeltaTime() const
{
	return CurrentNetworkTime - LastNetworkTime;
}

double UFGNetClock::GetNetAbsoluteTime() const
{
	checkf(ServerEpoch != -1.0, TEXT("ServerEpoch isn't yet replicated!"));
	return FApp::GetCurrentTime() - ServerEpoch;
}

double UFGNetClock::GetCurrentDriftRate() const
{
	return CurrentDrift;
}

double UFGNetClock::GetEpoch()
{
	return ServerEpoch;
}

void UFGNetClock::ClientSetEpoch_Implementation(double NewEpoch)
{
	ServerEpoch = NewEpoch;
	ReadyForUse = true;
}

void UFGNetClock::ServerRequestTime_Implementation(double T0)
{
	double T1 = GetNetAbsoluteTime();
	ClientUpdateTime(T0, T1);
}

void UFGNetClock::ClientUpdateTime_Implementation(double T1, double T2)
{
#if 0 // Vori's Clock
	const double T3 = GetNetAbsoluteTime();
	const double RTT = T3 - T1;
	double AdjustedRTT = 0;
	
	PreviousRTTs.Add(RTT);
	if (PreviousRTTs.Num() == 10)
	{
		TArray<double> SortedRTTs = PreviousRTTs;
		SortedRTTs.Sort();
		for (int32 Idx = 1; Idx < 9; ++Idx)
		{
			AdjustedRTT += SortedRTTs[Idx];
		}
		AdjustedRTT /= 8;
		PreviousRTTs.RemoveAt(0);
	}
	else
	{
		AdjustedRTT = RTT;
	}
	
	CurrentDrift = T2 - T1 - AdjustedRTT / 2.f;
#endif

	//const double T3 = GetNetAbsoluteTime();
	//const double ClientServerHalfPing = LastT0 - T1;
	//const double ServerClientHalfPing = T2 - T3;
	//CurrentDrift = ClientServerHalfPing + ServerClientHalfPing / 2.f;
}

//~ End NetClock
////////////////////////////////////////////////////////
