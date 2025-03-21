// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGGameSettingTypes.generated.h"

UENUM(BlueprintType)
enum class EFGSettingType : uint8
{
	Keybind,
	Slider,
	Stepped
};

UENUM(BlueprintType)
enum class EFGWindowModeSetting : uint8
{
	Fullscreen,
	Borderless,
	Windowed
};

UENUM(BlueprintType)
enum class EFGOverallGraphicsLevel : uint8
{
	Low,
	Medium,
	High,
	Ultra,
	Custom
};

UENUM(BlueprintType)
enum class EFGGraphicsLevel : uint8
{
	Low,
	Medium,
	High,
	Ultra
};

UENUM(BlueprintType)
enum class EFGNvidiaReflexLevel : uint8
{
	Off,
	On,
	Boost
};

UENUM(BlueprintType)
enum class EFGDLSSLevel : uint8
{
	Off,
	Performance,
	Balanced,
	Quality
};

UENUM(BlueprintType)
enum class EFGRHISetting : uint8
{
	DirectX11,
	DirectX12
};
