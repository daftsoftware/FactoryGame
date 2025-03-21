// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Components/ControllerComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "FGNetClock.generated.h"

////////////////////////////////////////////////////////
//~ Begin NetClock - Thx Vori <3

/*
	Provides network synchronization for local clocks using
	NTP (Network Time Protocol). You can read online about
	how this works however, the typical formula is this:
               	
    (T1 - T0) + (T2 - T3) / 2
        
	Where T0 is the time the packet was sent, T1 is the time the
	packet was received, T2 is the time the packet was sent back,
	and T3 is the time the packet was received back.

	NetClock offers automagic drift correction tm, you can use this
	to synchronize gameplay behaviour across clients and servers.
	(Even if said behaviour goes in reverse because of correction)

	Since this clock piggybacks on FApp::GetDeltaTime(), it is
	practically as precise as you will get on the engine, since the
	internal delta time is calculated using query performance counter
	or other platform specific high precision clocks.
*/
UCLASS()
class UFGNetClock final : public UControllerComponent
{
	GENERATED_BODY()

	UFGNetClock(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, DisplayName="Get Network Clock", Meta=(WorldContext="WorldContextObject", DefaultToSelf="WorldContextObject"))
	static UFGNetClock* Get(UObject* WorldContextObject);
	
	//~ Begin Super
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End Super

	// Epoch is set and we can use this clock.
	UFUNCTION(BlueprintPure)
	bool IsReadyForUse() const;

	/*
		Gets the delta time of the network clock. This can be
		negative in times of corrections! Use with caution.
		For high precision clock use FApp::GetDeltaTime().
	*/
	UFUNCTION(BlueprintPure)
	double GetNetDeltaTime() const;
	
	// Returns the absolute network time adjusted by drift.
	UFUNCTION(BlueprintPure)
	double GetNetAbsoluteTime() const;
	
	// Drift of this clock against the servers.
	UFUNCTION(BlueprintPure)
	double GetCurrentDriftRate() const;

	UFUNCTION(BlueprintPure)
	double GetEpoch();
	
	// Perform T0 and T1.
	UFUNCTION(Server, Unreliable)
	void ServerRequestTime(double T0);

	// Perform T2 and T3.
	UFUNCTION(Client, Unreliable)
	void ClientUpdateTime(double T1, double T2);

	double ServerEpoch = -1.0;

	UFUNCTION(Client, Reliable)
	void ClientSetEpoch(double NewEpoch);
	
	TArray<double> PreviousRTTs;
	
	double	NetworkClockUpdateFrequency		= 0.5;
	double	LastNetworkClockUpdate			= 0.5;
	
	double	LastNetworkTime					= 0.0;
	double	CurrentNetworkTime				= 0.0;
	double	CurrentDrift					= 0.0;
	bool	ReadyForUse						= false;

	// Temp
	double LastT0 = 0.0;
};

//~ End NetClock
////////////////////////////////////////////////////////
