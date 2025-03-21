// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGDeliveryDrone.h"
#include "FGVoxelUtils.h"
#include "System/FGAssetManager.h"
#include "Components/AudioComponent.h"
#include "Engine/StreamableManager.h"
#include "Logging/StructuredLog.h"

AFGDeliveryDrone::AFGDeliveryDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAlwaysPlay = true;
	AudioComponent->bAutoDestroy = false;
	AudioComponent->bAllowSpatialization = true;
}

void AFGDeliveryDrone::BeginPlay()
{
	Super::BeginPlay();
	
	TSoftObjectPtr<USoundBase> SoundToLoad = DroneSound;
	TSoftObjectPtr<USoundAttenuation> AttenuationToLoad = DroneAttenuation;
	FStreamableManager& StreamableMgr = UFGAssetManager::GetStreamableManager();
	
	StreamableMgr.RequestAsyncLoad(AttenuationToLoad.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
		[this, AttenuationToLoad]() {

		check(IsInGameThread());

		AudioComponent->AttenuationSettings = AttenuationToLoad.Get();
	}));
	
	StreamableMgr.RequestAsyncLoad(SoundToLoad.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
		[this, SoundToLoad]() {

		check(IsInGameThread());

		AudioComponent->SetSound(SoundToLoad.Get());
		AudioComponent->Play();
	}));
}

void AFGDeliveryDrone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector LastLocation = GetActorLocation();

	SetActorLocation(FMath::VInterpTo(
		GetActorLocation(),
		DeliveryVoxelLocation,
		DeltaSeconds,
		Speed));

	FVector Velocity = GetActorLocation() - LastLocation;

	float CurSpeed = Velocity.Size();

	float TargetPitch = FMath::GetMappedRangeValueClamped(
		FVector2D(0.f, 10.f),
		FVector2D(-2.f, 10.f),
		CurSpeed);

	CurrentPitch = FMath::FInterpTo(
		CurrentPitch,
		TargetPitch,
		DeltaSeconds,
		2.5f);

	UE_LOGFMT(LogTemp, Display, "Pitch: {Pitch}", CurrentPitch);

	AudioComponent->SetFloatParameter("PitchShift", CurrentPitch);
	
	if(CurSpeed < 10.f)
	{
		Velocity = FVector::ZeroVector;
	}
	
	Velocity.Normalize();

	UE_LOGFMT(LogTemp, Display, "Velocity: {Vel}", *Velocity.ToString());

	FRotator TargetRotation = FRotator(-Velocity.X * TiltStrength, 0.f, -Velocity.Y * TiltStrength);

	SetActorRotation(FMath::RInterpTo(
		GetActorRotation(),
		TargetRotation,
		DeltaSeconds,
		TiltSpeed));
}

void AFGDeliveryDrone::AttemptDelivery()
{
	// @TODO: Can we deliver in the target spot, try 3 times and then give up.
}

void AFGDeliveryDrone::DropPayload()
{
	// @TODO: Spawn a payload to drop at the delivery spot.
}