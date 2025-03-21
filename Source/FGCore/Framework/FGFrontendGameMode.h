// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover Alvarez

#pragma once

#include "GameFramework/GameModeBase.h"
#include "ControlFlowNode.h"
#include "FGFrontendGameMode.generated.h"

enum class ECommonUserOnlineContext : uint8;
enum class ECommonUserPrivilege : uint8;

class UUserWidget;
class UCommonUserInfo;

UCLASS()
class FGCORE_API AFGFrontendGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	//~AActor interface
	void BeginPlay() override;
	//~End of AActor interface

private:

	UFUNCTION()
	void OnUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

	void FlowStep_WaitForUserInitialization(FControlFlowNodeRef SubFlow);
	void FlowStep_TryShowPressStartScreen(FControlFlowNodeRef SubFlow);
	void FlowStep_TryJoinRequestedSession(FControlFlowNodeRef SubFlow);
	void FlowStep_TryShowMainScreen(FControlFlowNodeRef SubFlow);

	bool bShouldShowLoadingScreen = true;

	TSharedPtr<FControlFlow> FrontEndFlow;

	// If set, this is the in-progress press start screen task
	FControlFlowNodePtr InProgressPressStartScreen;

	FDelegateHandle OnJoinSessionCompleteEventHandle;
};
