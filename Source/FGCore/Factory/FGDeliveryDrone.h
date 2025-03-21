// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/Actor.h"
#include "FGDeliveryDrone.generated.h"

UCLASS()
class AFGDeliveryDrone : public AActor
{
	GENERATED_BODY()
public:

	AFGDeliveryDrone();

	//~ Begin Super
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;
	//~ End Super

	void AttemptDelivery();
	void DropPayload();

	UPROPERTY(EditDefaultsOnly)
	float Speed = 1.f;
	
	UPROPERTY(EditDefaultsOnly)
	float TiltSpeed = 3.f;
	
	UPROPERTY(EditDefaultsOnly)
	float TiltStrength = 25.f;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USoundBase> DroneSound;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USoundAttenuation> DroneAttenuation;
	
	UPROPERTY(EditAnywhere)
	FVector DeliveryVoxelLocation = FVector::ZeroVector;
	
	FVector CurrentVoxelLocation = FVector::ZeroVector;

	float CurrentPitch = 0.f;

private:

	int32 DeliveryAttempts = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> AudioComponent;
};
