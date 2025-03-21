// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGUserInterfaceStatics.h"

#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "UI/FGHUD.h"

AFGHUD* UFGUserInterfaceStatics::GetActiveHUD(UObject* WorldContext)
{
	if(UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		if(APlayerController* PC = World->GetFirstPlayerController())
		{
			if(AFGHUD* HUD = PC->GetHUD<AFGHUD>())
			{
				return HUD;
			}
		}
	}
	checkf(false, TEXT("Get active HUD returned null!"));
	return nullptr;
}

void UFGUserInterfaceStatics::AddWidgetToContentLayer(UObject* WorldContext, TSoftClassPtr<UFGActivatableWidget> Widget)
{
	if(AFGHUD* HUD = GetActiveHUD(WorldContext))
	{
		HUD->AddToContentLayer(Widget);
	}
}

void UFGUserInterfaceStatics::AddWidgetToMenuLayer(UObject* WorldContext, TSoftClassPtr<UFGActivatableWidget> Widget)
{
	if(AFGHUD* HUD = GetActiveHUD(WorldContext))
	{
		HUD->AddToMenuLayer(Widget);
	}
}

void UFGUserInterfaceStatics::AddWidgetToModalLayer(UObject* WorldContext, TSoftClassPtr<UFGModalWidget> Widget)
{
	if(AFGHUD* HUD = GetActiveHUD(WorldContext))
	{
		HUD->AddToModalLayer(Widget);
	}
}

UWidget* UFGUserInterfaceStatics::GetFocusedWidget()
{
	TSharedPtr<SWidget> FocusedSlateWidget = FSlateApplication::Get().GetUserFocusedWidget(0);
	if (!FocusedSlateWidget.IsValid())
	{
		//UE_LOG(LogTemp, Warning, TEXT("No focused Slate widget found"));
		return nullptr;
	}
	for (TObjectIterator<UWidget> Itr; Itr; ++Itr)
	{
		UWidget* CandidateUMGWidget = *Itr;
		if (CandidateUMGWidget->GetCachedWidget() == FocusedSlateWidget)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Focused UMG widget found: %s"), *CandidateUMGWidget->GetName());
			return CandidateUMGWidget;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("No focused UMG widget found"));
	return nullptr;
}