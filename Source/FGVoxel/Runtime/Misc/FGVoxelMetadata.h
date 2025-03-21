// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGVoxelDefines.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "FGVoxelMetadata.generated.h"

class AFGVoxelActor;

UCLASS()
class FGVOXEL_API UFGVoxelMetadata : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	UFGVoxelMetadata();

	//~ Begin Super
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~ End Super

	int32 GetVoxelType() const { return GVoxelTypeMap.FindChecked(VoxelName); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "EFGVoxelFlags"))
	int32 Flags;

	UPROPERTY(EditDefaultsOnly, meta=(Category="Voxel"))
	FGameplayTag VoxelName;

	UPROPERTY(EditDefaultsOnly, meta=(Category="Voxel"))
	TObjectPtr<UTexture2D> TopTexture;
	
	UPROPERTY(EditDefaultsOnly, meta=(Category="Voxel"))
	TObjectPtr<UTexture2D> BottomTexture;
	
	UPROPERTY(EditDefaultsOnly, meta=(Category="Voxel"))
	TObjectPtr<UTexture2D> FrontTexture;
	
	UPROPERTY(EditDefaultsOnly, meta=(Category="Voxel"))
	TObjectPtr<UTexture2D> BackTexture;
	
	UPROPERTY(EditDefaultsOnly, meta=(Category="Voxel"))
	TObjectPtr<UTexture2D> LeftTexture;
	
	UPROPERTY(EditDefaultsOnly, meta=(Category="Voxel"))
	TObjectPtr<UTexture2D> RightTexture;
	
	UPROPERTY(EditDefaultsOnly)
	TOptional<TSoftClassPtr<AFGVoxelActor>> ActorClass;
};
