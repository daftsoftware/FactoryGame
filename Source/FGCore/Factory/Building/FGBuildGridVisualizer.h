// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/Actor.h"
#include "FGBuildGridVisualizer.generated.h"

UCLASS()
class AFGBuildGridVisualizer : public AActor
{
	GENERATED_BODY()
public:

	AFGBuildGridVisualizer();
	
	//~ Begin Super
	void BeginPlay() override;
	void OnConstruction(const FTransform& Transform) override;
	//~ End Super

	void SetColor(FLinearColor Color);
	void RebuildGrid();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> RootScene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> GridPlanes;

private:

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> GridPlaneMesh;
};
