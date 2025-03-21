// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "GameFramework/LocalMessage.h"
#include "FGLocalMessage.generated.h"

UCLASS(Blueprintable, Abstract, NotPlaceable, meta = (ShowWorldContextPin))
class FGCORE_API UFGLocalMessage : public ULocalMessage
{
	GENERATED_BODY()
public:

};

UCLASS()
class FGCORE_API UFGChatMessage : public UFGLocalMessage
{
	GENERATED_BODY()
public:

	virtual void ClientReceiveChat(const FClientReceiveData& ClientData, FName Destination = NAME_None) const;
};