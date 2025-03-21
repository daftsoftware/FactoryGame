// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Engine/DeveloperSettings.h"
#include "FGVoxelProjectSettings.generated.h"

class AFGVoxelCollisionManager;
class UFGVoxelGenerator;
class AFGVoxelMesher;
class UStaticMesh;

UCLASS(Config=Game, DefaultConfig)
class FGVOXEL_API UFGVoxelProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UFGVoxelProjectSettings();

	//~ Begin Super
#if WITH_EDITORONLY_DATA
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End Super

	UPROPERTY(EditAnywhere, Config, Category = "Voxel")
	FString EditorWorldName = TEXT("DefaultWorld");

	UPROPERTY(EditAnywhere, Config, Category = "Voxel")
	TSoftClassPtr<AFGVoxelMesher> VoxelMesherClass;
	
	UPROPERTY(EditAnywhere, Config, Category = "Voxel")
	TSoftObjectPtr<UMaterialInterface> VoxelUberShader;
	
	UPROPERTY(EditAnywhere, Config, Category = "Voxel")
	TSoftObjectPtr<UMaterialInterface> PixelMeshShader;
	
	UPROPERTY(EditAnywhere, Config, Category = "Voxel")
	TSoftObjectPtr<UStaticMesh> VoxelISMMesh;

	UPROPERTY(EditAnywhere, Config, Category = "Voxel")
	TSoftClassPtr<UFGVoxelGenerator> VoxelDefaultGeneratorClass;
	
	UPROPERTY(EditAnywhere, Config, Category = "Voxel")
	TSoftObjectPtr<UMaterialInterface> MinimapPostProcessMaterial;

	UPROPERTY(EditAnywhere, Config, Category = "Voxel")
	TSoftClassPtr<AFGVoxelCollisionManager> VoxelDefaultCollisionManagerClass;
};
