// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameplayTagContainer.h"
#include "FGBuildRotationVisualizer.generated.h"

UCLASS()
class AFGBuildRotationVisualizer : public AActor
{
	GENERATED_BODY()
public:

	AFGBuildRotationVisualizer();

	//~ Begin Super
	void PostRegisterAllComponents() override;
	//~ End Super

	void UpdateAxis(FGameplayTagContainer PlayerModes);

	UPROPERTY()
	TObjectPtr<USceneComponent> RootScene;
	
	UPROPERTY(Transient)
	TMap<TEnumAsByte<EAxis::Type>, TObjectPtr<UStaticMeshComponent>> RingMeshes;
};
