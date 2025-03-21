// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGItemStack.h"

UFGItemStaticData::UFGItemStaticData()
	: DisplayName(FText::FromString("Invalid Name"))
{}

FPrimaryAssetId UFGItemStaticData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("Item"), *DisplayName.ToString());
}

// @TODO: Check on saving that display name is unique