// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover-Alvarez

#include "FGInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/FGLocalPlayer.h"
#include "Settings/FGSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FGInputComponent)

class UFGInputConfig;

UFGInputComponent::UFGInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UFGInputComponent::AddInputMappings(const UFGInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UFGInputComponent::RemoveInputMappings(const UFGInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UFGInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
