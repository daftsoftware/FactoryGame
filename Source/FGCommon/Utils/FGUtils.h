// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "FGUtils.generated.h"

namespace FG
{
	template<uint32 NumElems, typename T>
	TArray<T> MakeNumArrayElems(T Value)
	{
		static_assert(NumElems > 0, "NumElems must be greater than 0");

		TArray<T> OutArray;
		OutArray.Reserve(OutArray.Num() + NumElems);
		for(int32 Idx = 0; Idx < NumElems; Idx++)
		{
			OutArray.Add(Value);
		}
		return OutArray;
	}
}

/**
 * Factory Game Common Utilities Library.
 * Contains various useful conversion and helper functions.
 */
UCLASS(MinimalAPI)
class UFGUtils : public UObject
{
	GENERATED_BODY()
public:

	/**
	 * Gets the current camera transform in editor or game context.
	 * @param World - WorldContext to find camera in.
	 * @returns Transform of the camera, FTransform::Identity if unfound.
	 */
	UFUNCTION(BlueprintPure, Category="FactoryGame|Utility")
	static FGCOMMON_API FTransform GetCameraViewTransform(UWorld* World);

	UFUNCTION(BlueprintPure, Category="FactoryGame|Utility")
	static FGCOMMON_API FString GetGameVersion();
};