// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGItemSubsystem.h"
#include "Player/FGPlayerController.h"
#include "Player/FGPlayerPawn.h"
#include "System/FGAssetManager.h"
#include "FGItemStack.h"
#include "FGCoreMacros.h"
#include "Algo/Transform.h"

TArray<FName> UFGItemSubsystem::ListItemNames()
{
	TArray<FPrimaryAssetId> ItemIds;
	TArray<FName> OutItemNames;

	UFGAssetManager::Get().GetPrimaryAssetIdList(
		FPrimaryAssetType("Item"),
		ItemIds,
		EAssetManagerFilter::Default
	);

	Algo::Transform(ItemIds, OutItemNames, [](const FPrimaryAssetId& Id)
	{
		return Id.PrimaryAssetName;
	});
	return OutItemNames;
}

void UFGItemSubsystem::GiveItemByName(AFGPlayerController* PlayerController, FName ItemName, uint8 Count)
{
	if (!PlayerController)
	{
		UE_LOG(LogFG, Error, TEXT("GiveItemByName: PlayerController was NULL."));
		return;
	}

	TArray<FPrimaryAssetId> ItemIds;
	
	UFGAssetManager::Get().GetPrimaryAssetIdList(
		FPrimaryAssetType("Item"),
		ItemIds,
		EAssetManagerFilter::Default
	);

	TWeakObjectPtr<AFGPlayerPawn> Pawn = PlayerController->GetPawn<AFGPlayerPawn>();

	int32 AssetIdx = ItemIds.Find(FPrimaryAssetId(FPrimaryAssetType("Item"), ItemName));
	if(AssetIdx != INDEX_NONE) // Check we found an asset with ItemName.
	{
		TSoftObjectPtr<UFGItemStaticData> ItemData = TSoftObjectPtr<UFGItemStaticData>(
			UFGAssetManager::Get().GetPrimaryAssetPath(ItemIds[AssetIdx]));
		
		FStreamableManager& StreamableMgr = UFGAssetManager::GetStreamableManager();

		StreamableMgr.RequestAsyncLoad(ItemData.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, ItemData, Pawn, Count]()
		{
			check(IsInGameThread());
			if(Pawn.IsValid())
			{
				auto* ItemType = CastChecked<UFGItemStaticData>(ItemData.Get());
				int32 TargetSlot = Pawn->Backpack->AutoStack(FFGItemStack(ItemType, Count));

				if(TargetSlot != INDEX_NONE)
				{
					Pawn->Backpack->TransferToSlot(TargetSlot, FFGItemStack(ItemType, Count));					
					UE_LOG(LogFG, Display, TEXT("Gave player %s %s x%i"),
						*Pawn->GetName(), *ItemType->DisplayName.ToString(), Count);
				}
			}
		}));
	}
}

TArray<TSoftObjectPtr<UFGItemStaticData>> UFGItemSubsystem::GetAllItemDatas()
{
	TArray<FPrimaryAssetId> ItemIds;
	TArray<TSoftObjectPtr<UFGItemStaticData>> OutItemDatas;

	UFGAssetManager::Get().GetPrimaryAssetIdList(
		FPrimaryAssetType("Item"),
		ItemIds,
		EAssetManagerFilter::Default
	);

	Algo::Transform(ItemIds, OutItemDatas, [](const FPrimaryAssetId& Id)
	{
		return TSoftObjectPtr<UFGItemStaticData>(UFGAssetManager::Get().GetPrimaryAssetPath(Id));
	});
	return OutItemDatas;
}

void UFGItemSubsystem::GiveItem(AFGPlayerController* PlayerController, TSoftObjectPtr<UFGItemStaticData> ItemData, uint8 Count)
{
	if (!PlayerController)
	{
		UE_LOG(LogFG, Error, TEXT("GiveItem: PlayerController was NULL."));
		return;
	}
	
	TWeakObjectPtr<AFGPlayerPawn> Pawn = PlayerController->GetPawn<AFGPlayerPawn>();
	FStreamableManager& StreamableMgr = UFGAssetManager::GetStreamableManager();

	StreamableMgr.RequestAsyncLoad(ItemData.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
		[this, ItemData, Pawn, Count]()
	{
		check(IsInGameThread());
		if(Pawn.IsValid())
		{
			auto* ItemType = CastChecked<UFGItemStaticData>(ItemData.Get());
			int32 TargetSlot = Pawn->Backpack->AutoStack(FFGItemStack(ItemType, Count));

			if(TargetSlot != INDEX_NONE)
			{
				Pawn->Backpack->TransferToSlot(TargetSlot, FFGItemStack(ItemType, Count));
					
				UE_LOG(LogFG, Display, TEXT("Gave player %s %s x%i"),
					*Pawn->GetName(), *ItemType->DisplayName.ToString(), Count);
			}
		}
	}));
}

