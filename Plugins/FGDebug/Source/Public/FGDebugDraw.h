// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "UObject/Object.h"

/**
 * FactoryGame DebugDraw Utility functions offer a reasonable replacement
 * for stock "DrawDebugHelpers" and do reasonably the same thing functionally,
 * but they do it without nuking the performance of your game when you use them
 * at any sort of considerable scale.
 *
 * It primarily uses Niagara Systems and Niagara Data Channels in order
 * to do the rendering side. They are enabled / disabled in the same
 * contexts that DrawDebugHelpers are.
 */
namespace FG
{
	/**
	 * Draws a wireframe box in the world.
	 * @param World - World to draw the cube in.
	 * @param Location - Position to spawn the box at.
	 * @param Orientation - Rotation for the box.
	 * @param Scale - Uniform scale of the box in unreal units.
	 * @param Color - The color of the box.
	 * @param Lifetime - How long the box should last on the screen.
	 */
	FGDEBUG_API void DebugDrawBox(
		UWorld* World,
		FVector Location = FVector::ZeroVector,
		FQuat Orientation = FQuat::Identity,
		double Scale = 100.0, // @TODO: Make vector.
		FLinearColor Color = FLinearColor::Red,
		double Lifetime = 1.0);
}
	