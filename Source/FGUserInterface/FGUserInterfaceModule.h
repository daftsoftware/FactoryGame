// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Modules/ModuleInterface.h"

class FFGUserInterfaceModule : public IModuleInterface
{
	void StartupModule() override;
	void ShutdownModule() override;
	bool IsGameModule() const override { return false; }
};