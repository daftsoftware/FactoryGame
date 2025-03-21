// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGFrametimeDebugWidget.h"

#include "GenericPlatform/GenericPlatformTime.h"
#include "Components/TextBlock.h"

void UFGFrametimeDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	float UnitRaw = 0.f, UnitLo = TNumericLimits<float>::Max(), UnitAv = 0.f, UnitHi = 0.f, SampleTotal = 0.f;

	// Calc unit raw + add to sample set.
	UnitRaw = InDeltaTime * 1000.f; //GetOwningLocalPlayer()->ViewportClient->GetStatUnitData()->FrameTimes[0] * 1000.f;
	Samples.Insert(UnitRaw, 0);

	// Pop old samples off.
	while(Samples.Num() > NumSamples)
	{
		Samples.Pop();
	}

	// Loop samples to find lo, av and hi.
	for(int32 Idx = 0; Idx < Samples.Num(); Idx++)
	{
		SampleTotal += Samples[Idx];
		UnitHi = (Samples[Idx] > UnitHi) ? Samples[Idx] : UnitHi;
		UnitLo = (Samples[Idx] < UnitLo) ? Samples[Idx] : UnitLo;
	}

	// Calc unit av.
	UnitAv = SampleTotal / Samples.Num();

	// Format GUI
	MspfRaw->SetText(FText::FromString(FString::Printf(TEXT("mspf %.2f"), UnitRaw)));
	MspfLo->SetText(FText::FromString(FString::Printf(TEXT("lo %.2f"), UnitLo)));
	MspfAv->SetText(FText::FromString(FString::Printf(TEXT("av %.2f"), UnitAv)));
	MspfHi->SetText(FText::FromString(FString::Printf(TEXT("hi %.2f"), UnitHi)));	
}
