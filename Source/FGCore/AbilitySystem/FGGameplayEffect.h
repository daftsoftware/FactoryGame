// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameplayEffect.h"
#include "FGGameplayEffect.generated.h"

UCLASS()
class UFGGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Meta=(Category="GameplayEffect"))
	FGameplayTag EffectTag;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};