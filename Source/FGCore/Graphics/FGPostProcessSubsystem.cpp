// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGPostProcessSubsystem.h"
#include "..\System\FGAssetManager.h"
#include "..\System\FGGameData.h"

#include "Engine/PostProcessVolume.h"
#include "Engine/StreamableManager.h"
#include "Utils/FGUtils.h"

namespace FG
{
	bool DynamicDOFEnabled = true;
	FAutoConsoleVariableRef CVarDynamicDOFEnabled(
		TEXT("FG.DynamicDepthOfField"),
		DynamicDOFEnabled,
		TEXT("Enables or disables dynamic depth of field."),
		ECVF_Default
	);

	bool DynamicDOFInEditor = true;
	FAutoConsoleVariableRef CVarDynamicDOFInEditor(
		TEXT("FG.DynamicDepthOfFieldInEditor"),
		DynamicDOFInEditor,
		TEXT("Enables or disables dynamic depth of field in the editor."),
		ECVF_Default
	);

	float DynamicDOFNearDistance = 5000.f;
	FAutoConsoleVariableRef CVarDynamicDOFNearDistance(
		TEXT("FG.DynamicDepthOfFieldNearDistance"),
		DynamicDOFNearDistance,
		TEXT("The near distance for dynamic depth of field."),
		ECVF_Default
	);
	
	float DynamicDOFFarDistance = 10000.f;
	FAutoConsoleVariableRef CVarDynamicDOFFarDistance(
		TEXT("FG.DynamicDepthOfFieldFarDistance"),
		DynamicDOFFarDistance,
		TEXT("The far distance for dynamic depth of field."),
		ECVF_Default
	);
}

void UFGPostProcessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	PostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>();
	PostProcessVolume->SetActorScale3D(FVector::ZeroVector);
	PostProcessVolume->bUnbound = true;
	PostProcessVolume->bEnabled = true;
	PostProcessVolume->SetFlags(RF_Transient);
	PostProcessVolume->Rename(TEXT("DefaultMaterialPostProcess"), nullptr);

	for(TSoftObjectPtr<> Mat : UFGGameData::Get().DefaultPostProcessMaterials)
	{
		FStreamableManager& StreamableMgr = UFGAssetManager::GetStreamableManager();
       
       	StreamableMgr.RequestAsyncLoad(Mat.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
       		[this, Mat]()
       	{
       		check(IsInGameThread());

       		auto* Material = CastChecked<UMaterialInterface>(Mat.Get());
       		PostProcessVolume->AddOrUpdateBlendable(Material, 1.f);
       	}));
	}
}

void UFGPostProcessSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(FG::DynamicDOFEnabled)
	{
		FTransform CamXForm = UFGUtils::GetCameraViewTransform(GetWorld());
		FVector XDir = FRotationMatrix(CamXForm.Rotator()).GetScaledAxis(EAxis::X);
		
#if WITH_EDITOR
		if(GetWorld()->IsEditorWorld() && !FG::DynamicDOFInEditor)
		{
			return;
		}
#endif

		// @TODO: Dynamic DOF Implementation.
	}
}
