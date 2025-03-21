// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGHUDFrontend.h"
#include "System/FGGameData.h"

TSoftClassPtr<UFGLayoutWidget> AFGHUDFrontend::GetLayoutClass() const
{
	return UFGGameData::Get().FrontEndLayoutClass;
}
