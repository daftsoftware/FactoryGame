// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "FGItemSubsystem.generated.h"

class	AFGEquipmentInstance;
class	UFGItemStaticData;
class	AFGPlayerController;
struct	FFGItemAttributes;
struct	FFGItemStack;

/*
	FG Item Subsystem manages equipment pooling,
	item proxy representation, and bulk item processing.
*/
UCLASS()
class FGCORE_API UFGItemSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:

	TArray<FName>	ListItemNames();
	void			GiveItemByName(AFGPlayerController* PlayerController, FName ItemName, uint8 Count = 1);

	UFUNCTION(BlueprintPure)
	TArray<TSoftObjectPtr<UFGItemStaticData>> GetAllItemDatas();

	UFUNCTION(BlueprintCallable)
	void GiveItem(AFGPlayerController* PlayerController, TSoftObjectPtr<UFGItemStaticData> ItemData, uint8 Count = 1);
	
private:
	
	/*	Singleton actors per item type, on equipping said item type,
		it's pulled from the pool and attached to the player. */
	TMap<UFGItemStaticData*, AFGEquipmentInstance*> EquipmentPool;
};
