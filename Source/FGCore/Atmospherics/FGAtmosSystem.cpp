// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGAtmosSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"

AFGAtmosSystem::AFGAtmosSystem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	SkyAtmosphereComponent = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphereComponent"));
	SkyAtmosphereComponent->SetupAttachment(RootComponent);
	
	SunLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLightComponent"));
	SunLightComponent->SetupAttachment(RootComponent);
	
	MoonLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLightComponent"));
	MoonLightComponent->SetupAttachment(RootComponent);
	
	SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLightComponent"));
	SkyLightComponent->SetupAttachment(RootComponent);
	
	ExponentialHeightFogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFogComponent"));
	ExponentialHeightFogComponent->SetupAttachment(RootComponent);
	
	VolumetricCloudComponent = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloudComponent"));
	VolumetricCloudComponent->SetupAttachment(RootComponent);
}
