// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPerformanceSettings.h"

#include "Engine/PlatformSettingsManager.h"
#include "Misc/EnumRange.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGPerformanceSettings)

//////////////////////////////////////////////////////////////////////

const UFGPlatformSpecificRenderingSettings* UFGPlatformSpecificRenderingSettings::Get()
{
	UFGPlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
	check(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////

UFGPerformanceSettings::UFGPerformanceSettings()
{
	PerPlatformSettings.Initialize(UFGPlatformSpecificRenderingSettings::StaticClass());

	CategoryName = TEXT("Game");

	DesktopFrameRateLimits.Append({ 30, 60, 120, 144, 160, 165, 180, 200, 240, 360 });
}

