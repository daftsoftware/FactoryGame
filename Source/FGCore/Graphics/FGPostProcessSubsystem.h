// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "FGPostProcessSubsystem.generated.h"

class APostProcessVolume;

/*
	Post process handler that adds default game post process
	into the game and editor. For example any default post
	process materials like ScreenPasses &Depth of Field are
	best added here if they must exists in both game and editor
	contexts.

	You can also use this for game only effects, but it's recommended
	that you make sure it doesn't crash the editor.
*/
UCLASS()
class UFGPostProcessSubsystem final : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	//~ Begin Super
	void Initialize(FSubsystemCollectionBase& Collection) override;
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFGPostProcessSubsystem, STATGROUP_Tickables); }
	void Tick(float DeltaTime) override;
	//~ End Super

	UPROPERTY(Transient)
	TObjectPtr<APostProcessVolume> PostProcessVolume;
};
