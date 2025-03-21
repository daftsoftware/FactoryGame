// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Modules/ModuleInterface.h"

class FFGCommonModule : public IModuleInterface
{
	//~ Begin Super
	bool IsGameModule() const override { return false; }
	//~ End Super
};
