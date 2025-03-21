// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGAtmosSystem.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;
class UExponentialHeightFogComponent;
class UVolumetricCloudComponent;
class USkyAtmosphereComponent;

UCLASS()
class AFGAtmosSystem : public AActor
{
	GENERATED_BODY()
public:

	AFGAtmosSystem();

	UPROPERTY(VisibleAnywhere, Interp)
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphereComponent;
	
	UPROPERTY(VisibleAnywhere, Interp)
	TObjectPtr<UDirectionalLightComponent> SunLightComponent;

	UPROPERTY(VisibleAnywhere, Interp)
	TObjectPtr<UDirectionalLightComponent> MoonLightComponent;

	UPROPERTY(VisibleAnywhere, Interp)
	TObjectPtr<USkyLightComponent> SkyLightComponent;

	UPROPERTY(VisibleAnywhere, Interp)
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFogComponent;

	UPROPERTY(VisibleAnywhere, Interp)
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloudComponent;
};
