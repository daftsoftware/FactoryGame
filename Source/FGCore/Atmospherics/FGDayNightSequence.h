// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "LevelSequenceActor.h"
#include "FGDayNightSequence.generated.h"

UCLASS()
class AFGDayNightSequence final : public AReplicatedLevelSequenceActor
{
	GENERATED_BODY()
public:

	AFGDayNightSequence(const FObjectInitializer& Init);

	//~ Begin Super
	void PostRegisterAllComponents() override;
	//~ End Super

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	float GetSequenceEndSecs();
	float NormalizedTimeToSequenceSecs(float InTime);

	UPROPERTY(EditAnywhere, Category="FG")
	bool bPlayInEditor = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FG", Meta=(ClampMin="0.0"))
	float DayLengthMinutes = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FG", Meta=(ClampMin="0.0", ClampMax="1.0"))
	float InitialTimeOfDay = 0.0f;
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TSoftObjectPtr<UStaticMeshComponent> SkySphereMeshComponent;
};
