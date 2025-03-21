// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Logging/LogMacros.h"
#include "ProfilingDebugging/CsvProfiler.h"

/*----------------------------------------------------------------------------
	Logging & Printing
----------------------------------------------------------------------------*/

// Log Categories
FGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogFG, Log, All);
FGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogFGOnline, Log, All);
FGCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogFGAbilitySystem, Log, All);


// Easy usage wrapper for OnScreenDebugMessage / Print String Macro
#define UE_PRINT(text, ...) \
{ \
	FString LogPrefix = FString(__FUNCTION__); \
	FString OutPrint = LogPrefix + ": " + *FString::Printf(TEXT(text), ##__VA_ARGS__); \
	GEngine->AddOnScreenDebugMessage( \
	    - 1, \
	    10.f, \
	    FColor::White, \
	    *OutPrint); \
}

#define UE_PRINT_DURATION(duration,text,...) \
{ \
	FString LogPrefix = FString(__FUNCTION__); \
	FString OutPrint = LogPrefix + ": " + *FString::Printf(TEXT(text), ##__VA_ARGS__); \
	GEngine->AddOnScreenDebugMessage( \
	    - 1, \
	    duration, \
	    FColor::White, \
	    *OutPrint); \
}

/*----------------------------------------------------------------------------
	Profiling
----------------------------------------------------------------------------*/

// FG system profiler categories
namespace FGSysScope
{
	static const FLazyName Rendering		= FLazyName(TEXT("Rendering"));
	static const FLazyName Replication		= FLazyName(TEXT("Replication"));
	static const FLazyName Network			= FLazyName(TEXT("Network"));
	static const FLazyName Movement 		= FLazyName(TEXT("Movement"));
	static const FLazyName Physics			= FLazyName(TEXT("Physics"));
	static const FLazyName Animation		= FLazyName(TEXT("Animation"));
	static const FLazyName Pawn				= FLazyName(TEXT("Pawn"));
	static const FLazyName Grippables		= FLazyName(TEXT("Grippables"));
	static const FLazyName Gameplay			= FLazyName(TEXT("Gameplay"));
	static const FLazyName Misc				= FLazyName(TEXT("Misc"));
}

#if !UE_BUILD_SHIPPING

#define DEFINE_FG_PROFILE_CATEGORY(category) 		CSV_DEFINE_CATEGORY(category, true)
#define FG_PROFILE_SCOPED(category, statname)		CSV_SCOPED_TIMING_STAT(category, statname)

#endif