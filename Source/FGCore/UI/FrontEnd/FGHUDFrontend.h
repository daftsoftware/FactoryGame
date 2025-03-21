// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "UI/FGHUD.h"
#include "FGHUDFrontend.generated.h"

/*
	Glue class for any and all frontend related UI.
*/
UCLASS()
class FGCORE_API AFGHUDFrontend : public AFGHUD
{
	GENERATED_BODY()
public:

	//~ Begin Super
	TSoftClassPtr<UFGLayoutWidget> GetLayoutClass() const override;
	//~ End Super
	
};