// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelProjectSettings.h"
#include "World/FGVoxelSystem.h"

class UFGVoxelSystem;

UFGVoxelProjectSettings::UFGVoxelProjectSettings()
{
	CategoryName = TEXT("Game");	
}

#if WITH_EDITORONLY_DATA

void UFGVoxelProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// When renderer class changes, flush voxel data.
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGVoxelProjectSettings, VoxelMesherClass))
	{
		if(GWorld)
		{
			auto* VoxSys = GWorld->GetSubsystem<UFGVoxelSystem>();
			VoxSys->FlushRendering();
			VoxSys->InitializeRendering();
		}
	}
}

#endif
