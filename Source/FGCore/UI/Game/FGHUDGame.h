// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "UI/FGHUD.h"
#include "FGHUDGame.generated.h"

class UFGActivatableWidget;
class UFGUISettings;
class UFGModalWidget;
class UFGLayoutWidget;
class UFGGameContentWidget;
class UFGLocalMessage;

/*
	The HUD is the centralized manager and glue for all UI.
*/
UCLASS()
class FGCORE_API AFGHUDGame : public AFGHUD
{
	GENERATED_BODY()
public:

	AFGHUDGame();
	
	//~ Begin Super
	virtual void BeginPlay() override;
	virtual void SetupInputComponent(UInputComponent* HUDInputComponent);
	virtual TSoftClassPtr<UFGLayoutWidget> GetLayoutClass() const override;
	//~ End Super

	UFUNCTION(BlueprintCallable, Category="UI")
	void OpenEscapeMenu();
	
	UFUNCTION(BlueprintCallable, Category="UI")
	void OpenInventory();

	UFUNCTION(BlueprintCallable, Category="UI")
	void OpenSettings();

	// Receive a localized message from somewhere to be displayed
	virtual void ReceiveLocalMessage(TSubclassOf<UFGLocalMessage> MessageClass, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, FName MessageType, FText MessageText, UObject* OptionalObject = nullptr);

protected:

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UFGGameContentWidget* GameUI;

	UPROPERTY(Transient)
	TObjectPtr<UFGActivatableWidget> EscapeMenu;
	
	UPROPERTY(Transient)
	TObjectPtr<UFGActivatableWidget> InventoryMenu;

	UPROPERTY(Transient)
	TObjectPtr<UFGUISettings> SettingsMenu;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFGActivatableWidget>> CustomMenus;
};