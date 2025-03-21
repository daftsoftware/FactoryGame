// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/Actor.h"
#include "FGPixelMeshDebugger.generated.h"

class UDynamicMeshComponent;

UCLASS()
class FGCORE_API AFGPixelMeshDebugger : public AActor
{
	GENERATED_BODY()
public:

	AFGPixelMeshDebugger();

	//~ Begin Super
	void PostInitializeComponents() override;
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End Super

	UPROPERTY(EditAnywhere)
	UTexture2D* Texture2D;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDynamicMeshComponent> DynMeshComponent;
};
