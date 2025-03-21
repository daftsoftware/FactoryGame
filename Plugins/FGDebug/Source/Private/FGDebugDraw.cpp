// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGDebugDraw.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannelAccessor.h"

namespace FG
{
	static TSoftObjectPtr<UNiagaraDataChannelAsset> BoxChannelAsset = TSoftObjectPtr<UNiagaraDataChannelAsset>(
		FSoftObjectPath("/FGDebug/NDC_FGDebugBox.NDC_FGDebugBox"));

	void DebugDrawBox(UWorld* World, FVector Location, FQuat Orientation, double Scale, FLinearColor Color, double Lifetime)
	{
#if UE_ENABLE_DEBUG_DRAWING
		UNiagaraDataChannelAsset* LoadedChannel = BoxChannelAsset.LoadSynchronous();
		checkf(LoadedChannel, TEXT("Invalid Debug Box Channel! Missing Plugin Content?"));

		FNiagaraDataChannelSearchParameters SearchParams;
		SearchParams.Location = Location;
		
		UNiagaraDataChannelWriter* Writer = UNiagaraDataChannelLibrary::WriteToNiagaraDataChannel(
			World,
			LoadedChannel,
			SearchParams,
			1,
			true,
			true,
			true,
			TEXT("FactoryGame - BoxDebugDraw")
		);

		Writer->WriteFloat("Lifetime", 0, Lifetime);
		Writer->WriteLinearColor("Color", 0, Color);
		Writer->WritePosition("Location", 0, Location);
		Writer->WriteQuat("Orientation", 0, Orientation);
		Writer->WriteVector("Scale", 0, FVector(Scale));
#endif
	}
}
