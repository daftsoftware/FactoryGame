// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelGenerator.h"
#include "Containers/FGVoxelGrid.h"

UFGVoxelGrid* UFGVoxelGenerator::GetOwningVoxelGrid() const
{
	return CastChecked<UFGVoxelGrid>(GetOuter());
}