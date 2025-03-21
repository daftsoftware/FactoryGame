// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Alvaro Jover Alvarez

#include "FGFrontendGameMode.h"
#include "FGGameInstance.h"
#include "..\System\FGGameData.h"

#include "CommonSessionSubsystem.h"
#include "CommonUserSubsystem.h"
#include "ControlFlowManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/FGLayoutWidget.h"

void AFGFrontendGameMode::BeginPlay()
{
	Super::BeginPlay();

	FControlFlow& Flow = FControlFlowStatics::Create(this, TEXT("FrontendFlow"))
		.QueueStep(TEXT("Wait For User Initialization"), this, &ThisClass::FlowStep_WaitForUserInitialization)
		.QueueStep(TEXT("Try Show Press Start Screen"), this, &ThisClass::FlowStep_TryShowPressStartScreen)
		.QueueStep(TEXT("Try Join Requested Session"), this, &ThisClass::FlowStep_TryJoinRequestedSession)
		.QueueStep(TEXT("Try Show Main Screen"), this, &ThisClass::FlowStep_TryShowMainScreen);

	Flow.ExecuteFlow();

	FrontEndFlow = Flow.AsShared();
}

void AFGFrontendGameMode::OnUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	FControlFlowNodePtr FlowToContinue = InProgressPressStartScreen;
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

	if (ensure(FlowToContinue.IsValid() && UserSubsystem))
	{
		UserSubsystem->OnUserInitializeComplete.RemoveDynamic(this, &ThisClass::OnUserInitialized);
		InProgressPressStartScreen.Reset();

		if (bSuccess)
		{
			// On success continue flow normally
			FlowToContinue->ContinueFlow();
		}
		else
		{
			// TODO: Just continue for now, could go to some sort of error screen
			FlowToContinue->ContinueFlow();
		}
	}
}

void AFGFrontendGameMode::FlowStep_WaitForUserInitialization(FControlFlowNodeRef SubFlow)
{
	// If this was a hard disconnect, explicitly destroy all user and session state
	// TODO: Refactor the engine disconnect flow so it is more explicit about why it happened
	bool bWasHardDisconnect = false;
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);

	if (ensure(GameMode) && UGameplayStatics::HasOption(GameMode->OptionsString, TEXT("closed")))
	{
		bWasHardDisconnect = true;
	}

	// Only reset users on hard disconnect
	UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
	if (ensure(UserSubsystem) && bWasHardDisconnect)
	{
		UserSubsystem->ResetUserState();
	}

	// Always reset sessions
	UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
	if (ensure(SessionSubsystem))
	{
		SessionSubsystem->CleanUpSessions();
	}

	SubFlow->ContinueFlow();
}

void AFGFrontendGameMode::FlowStep_TryShowPressStartScreen(FControlFlowNodeRef SubFlow)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

	// Check to see if the first player is already logged in, if they are, we can skip the press start screen.
	if (const UCommonUserInfo* FirstUser = UserSubsystem->GetUserInfoForLocalPlayerIndex(0))
	{
		if (FirstUser->InitializationState == ECommonUserInitializationState::LoggedInLocalOnly ||
			FirstUser->InitializationState == ECommonUserInitializationState::LoggedInOnline)
		{
			SubFlow->ContinueFlow();
			return;
		}
	}

	// TODO: For console support - Ensure through certification that we are fine logging always Player 0
	// otherwise we need to show a start screen to detect which one of the two controllers pressed X to 
	// login the appropriate one.

	// Start the auto login process, this should finish quickly and will use the default input device id
	InProgressPressStartScreen = SubFlow;
	UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ThisClass::OnUserInitialized);
	UserSubsystem->TryToInitializeForLocalPlay(0, FInputDeviceId(), false);	
}

void AFGFrontendGameMode::FlowStep_TryJoinRequestedSession(FControlFlowNodeRef SubFlow)
{
	UFGGameInstance* GameInstance = Cast<UFGGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GameInstance->GetRequestedSession() != nullptr && GameInstance->CanJoinRequestedSession())
	{
		UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
		if (ensure(SessionSubsystem))
		{
			// Bind to session join completion to continue or cancel the flow
			// TODO:  Need to ensure that after session join completes, the server travel completes.
			OnJoinSessionCompleteEventHandle = SessionSubsystem->OnJoinSessionCompleteEvent.AddWeakLambda(this, [this, SubFlow, SessionSubsystem](const FOnlineResultInformation& Result)
				{
					// Unbind delegate. SessionSubsystem is the object triggering this event, so it must still be valid.
					SessionSubsystem->OnJoinSessionCompleteEvent.Remove(OnJoinSessionCompleteEventHandle);
					OnJoinSessionCompleteEventHandle.Reset();

					if (Result.bWasSuccessful)
					{
						// No longer transitioning to the main menu
						SubFlow->CancelFlow();
					}
					else
					{
						// Proceed to the main menu
						SubFlow->ContinueFlow();
						return;
					}
				});
			GameInstance->JoinRequestedSession();
			return;
		}
	}
	// Skip this step if we didn't start requesting a session join
	SubFlow->ContinueFlow();
}

void AFGFrontendGameMode::FlowStep_TryShowMainScreen(FControlFlowNodeRef SubFlow)
{
	// @TODO: Eval - Disconnected for now 11/05/24 sunny
	// I made FGHUDFrontend initialize our menu instead but we might want
	// to use this for something?
#if 0
	// Instantiate the main UI widget.
	check(!UFGGameData::Get().FrontEndLayoutClass.IsNull()); // No Main Menu Widget set.

	// Instantiate the main UI widget.
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);

	UFGLayoutWidget* MainMenuWidget = CreateWidget<UFGLayoutWidget>(
		GameInstance->GetFirstLocalPlayerController(), UFGGameData::Get().FrontEndLayoutClass.LoadSynchronous());

	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport(0);
	}
#endif
}
