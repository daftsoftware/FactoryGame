// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelModule.h"

#include "Modules/ModuleManager.h"
#include "Misc/CoreDelegates.h"

void FFGVoxelModule::StartupModule()
{
	//const FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	//AddShaderSourceDirectoryMapping(TEXT("/Project"), ShaderDirectory);

#if 0
	FCoreDelegates::OnPostEngineInit.AddLambda([this]()
	{
		check(!VoxelRenderExt.IsValid());
		VoxelRenderExt = FSceneViewExtensions::NewExtension<FFGVoxelViewExtension>();
	});
	
	FCoreDelegates::OnEnginePreExit.AddLambda([this]()
	{
		check(VoxelRenderExt.IsValid());
		VoxelRenderExt.Reset();
	});
#endif
}

IMPLEMENT_MODULE(FFGVoxelModule, FGVoxel);