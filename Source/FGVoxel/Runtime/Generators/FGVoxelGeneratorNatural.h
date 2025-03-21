// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGVoxelGenerator.h"
#include "FGVoxelGeneratorNatural.generated.h"

UCLASS()
class UFGVoxelGeneratorNatural : public UFGVoxelGenerator
{
	GENERATED_BODY()
public:
	
	//~ Begin Super
	virtual void Generate(TArray<FFGVoxelChunk>& ChunkData, FFGChunkHandle ChunkHandle) override;
	//~ End Super
	
};
