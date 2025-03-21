// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "ExtendableGameStateBase.h"
#include "FGGameState.generated.h"

/*
	FGGameStateBase.
*/
UCLASS()
class FGCORE_API AFGGameState : public AExtendableGameStateBase
{
	GENERATED_BODY()
public:

	AFGGameState();
	
	//~ Begin Super
	void Tick(float DeltaSeconds) override;
	//~ End Super
};
