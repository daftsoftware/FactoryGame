// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FastNoise2Module.h"
#include "Interfaces/IPluginManager.h"

void FFastNoise2Module::StartupModule()
{
	const FString BaseDir = IPluginManager::Get().FindPlugin("FastNoise2")->GetBaseDir();
	const FString LibraryPath = FPaths::Combine(*BaseDir, TEXT(FASTNOISE_LIBRARY_PATH));

	FastNoiseHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (FastNoiseHandle == nullptr)
	{
		const FText ErrorFormat = NSLOCTEXT("FastNoise2Module", "ThirdPartyLibraryError", "Failed to load FastNoise library at path [{0}]");
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(ErrorFormat, FText::FromString(LibraryPath)));
	}
}

void FFastNoise2Module::ShutdownModule()
{
	FPlatformProcess::FreeDllHandle(FastNoiseHandle);
	FastNoiseHandle = nullptr;
}

IMPLEMENT_MODULE(FFastNoise2Module, FastNoise2)