// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGCoreMacros.h"

/*----------------------------------------------------------------------------
	Logging & Printing
----------------------------------------------------------------------------*/

// Log Categories
DEFINE_LOG_CATEGORY(LogFG);
DEFINE_LOG_CATEGORY(LogFGOnline);
DEFINE_LOG_CATEGORY(LogFGAbilitySystem);

/*----------------------------------------------------------------------------
	Profiling
----------------------------------------------------------------------------*/

#if !UE_BUILD_SHIPPING

DEFINE_FG_PROFILE_CATEGORY(FGRendering);
DEFINE_FG_PROFILE_CATEGORY(FGReplication);
DEFINE_FG_PROFILE_CATEGORY(FGNetwork);
DEFINE_FG_PROFILE_CATEGORY(FGMovement);
DEFINE_FG_PROFILE_CATEGORY(FGPhysics);
DEFINE_FG_PROFILE_CATEGORY(FGAnimation);
DEFINE_FG_PROFILE_CATEGORY(FGPawn);
DEFINE_FG_PROFILE_CATEGORY(FGGrippables);
DEFINE_FG_PROFILE_CATEGORY(FGGameplay);
DEFINE_FG_PROFILE_CATEGORY(FGMisc);

#endif