// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "FGThumbnailRenderer.generated.h"

class USceneCaptureComponent2D;

UCLASS()
class FGCORE_API AFGThumbnailRenderer final : public AActor
{
	GENERATED_BODY()
public:

	AFGThumbnailRenderer();

	void BuildFlipbookAtlas();

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> MeshPool;
	
	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> StripCapture;
};

UCLASS()
class FGCORE_API UFGThumbnailSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()
public:

	//~ Begin Super
	virtual void PostInitialize() override;
	//~ End Super

private:

	UPROPERTY()
	TObjectPtr<AFGThumbnailRenderer> ThumbnailRenderer;
};
