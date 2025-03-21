// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Modules/ModuleInterface.h"

class FFGVoxelModule : public IModuleInterface
{
	//~ Begin Super
	void StartupModule() override;
	bool IsGameModule() const override { return false; }
	//~ End Super
};
