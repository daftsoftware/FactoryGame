// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGGameplayEffect.h"

FPrimaryAssetId UFGGameplayEffect::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("GameplayEffect"), EffectTag.GetTagName());
}