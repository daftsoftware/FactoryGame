// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Blueprint/UserWidget.h"
#include "FGFrametimeDebugWidget.generated.h"

class UTextBlock;

UCLASS()
class UFGFrametimeDebugWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	//~ Begin Super
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End Super

	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	UTextBlock* MspfRaw;

	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	UTextBlock* MspfLo;
	
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	UTextBlock* MspfAv;

	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	UTextBlock* MspfHi;

private:

	uint8 NumSamples = 8;
	TArray<float> Samples;
};
