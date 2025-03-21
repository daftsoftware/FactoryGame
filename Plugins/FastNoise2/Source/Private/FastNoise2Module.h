// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Modules/ModuleInterface.h"

class FFastNoise2Module final : public IModuleInterface
{
public:
	//~ Begin Super
	void StartupModule() override;
	void ShutdownModule() override;
	//~ End Super
	
private:
	void* FastNoiseHandle = nullptr;
};