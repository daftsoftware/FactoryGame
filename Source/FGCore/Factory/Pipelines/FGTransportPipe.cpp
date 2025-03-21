// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGTransportPipe.h"
#include "FGTransportPipeManager.h"

void AFGTransportPipe::BeginPlay()
{
	Super::BeginPlay();

	auto* PipeManager = GetWorld()->GetSubsystem<UFGTransportPipeManager>();
	PipeManager->RegisterNode(this);
}

void AFGTransportPipe::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	auto* PipeManager = GetWorld()->GetSubsystem<UFGTransportPipeManager>();
	PipeManager->UnregisterNode(this);

	Super::EndPlay(EndPlayReason);
}
