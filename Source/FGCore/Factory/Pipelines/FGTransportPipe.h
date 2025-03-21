// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "World/FGVoxelActorManager.h"
#include "FGTransportPipe.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFGTransportPipeDirections : uint8
{
	None,
	Forward		= 1 << 0,
	Backward	= 1 << 1,
	Left		= 1 << 2,
	Right		= 1 << 3,
	Up			= 1 << 4,
	Down		= 1 << 5
};
ENUM_CLASS_FLAGS(EFGTransportPipeDirections)

UCLASS()
class AFGTransportPipe : public AFGVoxelActor
{
	GENERATED_BODY()
public:

	//~ Begin Super
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End Super

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "EFGTransportPipeDirections"))
	int32 ActiveEndpoints;

	UFUNCTION(BlueprintImplementableEvent)
	void RefreshMesh();
};
