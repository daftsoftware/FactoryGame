// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "Modules/ModuleInterface.h"

class FFGDebugModule : public IModuleInterface {};

IMPLEMENT_MODULE(FFGDebugModule, FGDebug)