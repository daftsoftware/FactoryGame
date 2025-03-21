// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "World/FGVoxelActorManager.h"
#include "FGQuarry.generated.h"

class USplineMeshComponent;
class UInstancedStaticMeshComponent;

USTRUCT(BlueprintType)
struct FFGQuarryRegion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FIntVector Min = FIntVector::ZeroValue;
	
	UPROPERTY(EditAnywhere)
	FIntVector Max = FIntVector(32, 32, 8);
};

/**
 * Standard plot quarry block, a region can be marked out and then
 * this quarry block will build and maintain the quarry region, digging
 * out any voxels within the region, down to bedrock.
 */
UCLASS()
class AFGQuarry : public AFGVoxelActor
{
	GENERATED_BODY()
public:

	AFGQuarry();

	//~ Begin Super
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End Super

	void SetupFrame();
	
	UPROPERTY(EditAnywhere)
	FFGQuarryRegion DefaultRegion;

	UPROPERTY(EditAnywhere)
	double DigRateSeconds = 0.01f;

	UPROPERTY(EditAnywhere)
	int32 MaximumDepth = 64;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> QuarryFrameMeshAsset;
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> QuarryFrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USplineMeshComponent> QuarryHeadSupportMeshX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USplineMeshComponent> QuarryHeadSupportMeshY;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USplineMeshComponent> QuarryDrillBitMesh;
	
	FIntVector HeadOffset = FIntVector::ZeroValue;
	double DigCooldown = DigRateSeconds;
	
	bool RegionCleared = false;
	bool SetupFinished = false;
	
	FVector CurrentHeadLocation = FVector::ZeroVector;
	FVector TargetHeadLocation = FVector::ZeroVector;
};
