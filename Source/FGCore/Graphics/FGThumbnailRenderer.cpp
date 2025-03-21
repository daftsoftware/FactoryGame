// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGThumbnailRenderer.h"
#include "Components/SceneCaptureComponent2D.h"

AFGThumbnailRenderer::AFGThumbnailRenderer()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	StripCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("StripCapture"));
	StripCapture->SetupAttachment(RootComponent);

	StripCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	StripCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	StripCapture->ShowFlags.DisableAdvancedFeatures();
	StripCapture->ShowFlags.SetFog(false);
	StripCapture->ShowFlags.SetAtmosphere(false);
	StripCapture->ShowFlags.SetTranslucency(true);
	StripCapture->ShowFlags.SetInstancedStaticMeshes(false);
	StripCapture->ShowFlags.SetSkeletalMeshes(false);
	StripCapture->ShowFlags.SetStaticMeshes(true);
	StripCapture->ShowFlags.SetAntiAliasing(true);
	StripCapture->ShowFlags.SetLighting(false);
	StripCapture->ShowFlags.SetGlobalIllumination(false);
	StripCapture->ShowFlags.SetPostProcessing(true);
	StripCapture->ShowFlags.SetPostProcessMaterial(true);

#if WITH_EDITORONLY_DATA
	bListedInSceneOutliner = false;
#endif
}

void AFGThumbnailRenderer::BuildFlipbookAtlas()
{
	StripCapture->ClearShowOnlyComponents();

	TArray<TWeakObjectPtr<UPrimitiveComponent>> WeakPrimPool;
	Algo::Transform(MeshPool, WeakPrimPool, [](UStaticMeshComponent* Mesh)
	{
		return MakeWeakObjectPtr<UPrimitiveComponent>(Mesh);
	});

	StripCapture->ShowOnlyComponents = WeakPrimPool;
}

void UFGThumbnailSubsystem::PostInitialize()
{
	Super::PostInitialize();

	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |=	RF_Transient;
	SpawnParams.ObjectFlags &= ~RF_Transactional;
	
	ThumbnailRenderer = GetWorld()->SpawnActor<AFGThumbnailRenderer>(SpawnParams);
}