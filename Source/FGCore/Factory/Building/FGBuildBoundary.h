// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/Actor.h"
#include "FGBuildBoundary.generated.h"

class UBoxComponent;

UCLASS()
class AFGBuildBoundary : public AActor
{
	GENERATED_BODY()
public:

	AFGBuildBoundary();
	
	//~ Begin Super
	void OnConstruction(const FTransform& Transform) override;
	//~ End Super

	UPROPERTY(EditAnywhere, Category="FG")
	int32 GridSizeSquared = 1;

	UPROPERTY(EditAnywhere, Category="FG")
	float BoundryHeight = 5000.f;
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> BoundryRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> VolumeComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> DesaturationBoundryMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> GridBoundryMeshComponent;
};
