// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "UObject/Object.h"
#include "FGVoxelKismetUtils.generated.h"

UCLASS()
class UFGVoxelKismetUtils : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Meta = (HidePin = "WorldContext", DefaultToSelf ="WorldContext"))
	static int32 GetVoxelAtLocation(UObject* WorldContext, FVector Location);
};
