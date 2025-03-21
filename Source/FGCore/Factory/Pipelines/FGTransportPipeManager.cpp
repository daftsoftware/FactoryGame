// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGTransportPipeManager.h"
#include "FGTransportPipe.h"
#include "FGVoxelUtils.h"

static const FIntVector	DOFMaskTable[6] = 
{
	FIntVector( 1,  0,  0),	// Forward
	FIntVector(-1,  0,  0), // Back
	FIntVector( 0,  1,  0), // Left
	FIntVector( 0, -1,  0), // Right
	FIntVector( 0,  0,  1), // Up
	FIntVector( 0,  0, -1)  // Down
};

void UFGTransportPipeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void UFGTransportPipeManager::RegisterNode(AFGTransportPipe* NewNode)
{
	FIntVector VoxelCoordinate = FIntVector(NewNode->GetActorLocation() / FG::Const::VoxelSizeUU);
	NodeMap.Add(VoxelCoordinate, NewNode);
	
	for(auto Node : NodeMap)
	{
		VoxelCoordinate = FIntVector(Node.Value->GetActorLocation() / FG::Const::VoxelSizeUU);
		for(int32 DOF = 0; DOF < 6; DOF++)
		{
			FIntVector NeighbourCoordinate = VoxelCoordinate + DOFMaskTable[DOF];

			if(NodeMap.Contains(NeighbourCoordinate))
			{
				DrawDebugBox(
					GetWorld(),
					FVector(NeighbourCoordinate * FG::Const::VoxelSizeUU) + FVector(FG::Const::VoxelSizeUU / 2),
					FVector(FG::Const::VoxelSizeUU / 2),
					FColor::Green,
					false,
					1.0f);
				
				Node.Value->ActiveEndpoints |= 1 << DOF;
			}
		}
		
		Node.Value->RefreshMesh();
	}
}

void UFGTransportPipeManager::UnregisterNode(AFGTransportPipe* OldNode)
{
	FIntVector VoxelCoordinate = FIntVector(OldNode->GetActorLocation() / FG::Const::VoxelSizeUU);
	NodeMap.Remove(VoxelCoordinate);
	
	for(auto Node : NodeMap)
	{
		VoxelCoordinate = FIntVector(Node.Value->GetActorLocation() / FG::Const::VoxelSizeUU);

		for(int32 DOF = 0; DOF < 6; DOF++)
		{
			FIntVector NeighbourCoordinate = VoxelCoordinate + DOFMaskTable[DOF];

			if(NodeMap.Contains(NeighbourCoordinate))
			{
				DrawDebugBox(
					GetWorld(),
					FVector(NeighbourCoordinate * FG::Const::VoxelSizeUU) + FVector(FG::Const::VoxelSizeUU / 2),
					FVector(FG::Const::VoxelSizeUU / 2),
					FColor::Green,
					false,
					1.0f);
				
				Node.Value->ActiveEndpoints |= 1 << DOF;
			}
		}
		
		Node.Value->RefreshMesh();
	}
}