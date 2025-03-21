// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "CommonActivatableWidget.h"
#include "FGActivatableWidget.generated.h"

class UInputAction;

UENUM(BlueprintType)
enum class EFGWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

UCLASS()
class FGUSERINTERFACE_API UFGActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
public:

	//~ Begin Super
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~ End Super
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* BackHandlerOverride;

	FUIActionBindingHandle CustomBackHandler;
	
protected:

	/*
		The desired input mode to use while this UI is activated, for example
		do you want key presses to still reach the game/player controller?
	*/
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EFGWidgetInputMode InputConfig = EFGWidgetInputMode::Default;

	// The desired mouse behavior when the game gets input.
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;	
};
