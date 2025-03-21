// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelDefines.h"

TMap<FGameplayTag, int32> GVoxelTypeMap {};
Experimental::TRobinHoodHashMap<int32, EFGVoxelFlags> GVoxelTypeFlagMap {};
