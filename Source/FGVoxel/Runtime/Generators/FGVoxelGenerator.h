// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Containers/FGVoxelChunk.h"
#include "FGVoxelGenerator.generated.h"

struct FFGChunkHandleData;
using FFGChunkHandle = TSharedPtr<FFGChunkHandleData>;

UCLASS()
class UFGVoxelGenerator : public UObject
{
	GENERATED_BODY()
public:
	
	virtual void Generate(TArray<FFGVoxelChunk> &ChunkData, FFGChunkHandle ChunkHandle) {}
	UFGVoxelGrid* GetOwningVoxelGrid() const;
};
