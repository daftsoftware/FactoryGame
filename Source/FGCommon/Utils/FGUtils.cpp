// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGUtils.h"

#include <chrono>

#if WITH_EDITOR
	#include "EditorViewportClient.h"
#endif

FTransform UFGUtils::GetCameraViewTransform(UWorld* World)
{
	checkf(IsInGameThread(), TEXT("GetCameraViewTransform must be called on the game thread."));

	// Server incorrectly returns the camera position of the first player.
	if(!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return FTransform::Identity;
	}

	if(World->IsGameWorld()) // Game Context.
	{
		if(APlayerController* PC = World->GetFirstPlayerController()) 
		{
			checkf(PC->PlayerCameraManager, TEXT("Player Camera Manager is null!"));

			return FTransform(
				PC->PlayerCameraManager->GetCameraRotation(),
				PC->PlayerCameraManager->GetCameraLocation(),
				FVector::OneVector
			);
		}
		return FTransform::Identity;
	}
	else // Editor Context.
	{
#if WITH_EDITOR
		TArray<const FEditorViewportClient*, TInlineAllocator<8>> ValidClients;
		for(const FEditorViewportClient* ViewportClient : GEditor->GetAllViewportClients())
		{
			if(ViewportClient->GetWorld() == World)
			{
				ValidClients.Add(ViewportClient);
			}
		}

		const FViewport* ActiveViewport = GEditor->GetActiveViewport();
		const FEditorViewportClient* ActiveViewportClient = nullptr;
	
		for(const FEditorViewportClient* ViewportClient : ValidClients)
		{
			if(ViewportClient->Viewport == ActiveViewport)
			{
				ActiveViewportClient = ViewportClient;
				break;
			}
		}

		if(ActiveViewportClient)
		{
			if(!ActiveViewportClient->IsPerspective())
			{
				for(const FEditorViewportClient* ViewportClient : ValidClients)
				{
					if(ViewportClient->IsPerspective())
					{
						ActiveViewportClient = ViewportClient;
						break;
					}
				}
			}
			return FTransform(
				ActiveViewportClient->GetViewRotation(),
				ActiveViewportClient->GetViewLocation(),
				FVector::OneVector
			);
		}

		if(ValidClients.Num() > 0)
		{
			return FTransform(
				ValidClients.Last()->GetViewRotation(),
				ValidClients.Last()->GetViewLocation(),
				FVector::OneVector
			);
		}
#endif
		return FTransform::Identity;
	}
}

consteval std::chrono::year_month_day BuildCompileDate() {
	using namespace std::chrono;
	constexpr auto m2i = [](const char* s) { return (s[0] << 16) | (s[1] << 8) | s[2]; };
	constexpr auto d   = [](int i) { return __DATE__[i] - '0'; };
	month          m;
	switch (m2i(__DATE__)) {
	case m2i("Jan"): m = January; break;
	case m2i("Feb"): m = February; break;
	case m2i("Mar"): m = March; break;
	case m2i("Apr"): m = April; break;
	case m2i("May"): m = May; break;
	case m2i("Jun"): m = June; break;
	case m2i("Jul"): m = July; break;
	case m2i("Aug"): m = August; break;
	case m2i("Sep"): m = September; break;
	case m2i("Oct"): m = October; break;
	case m2i("Nov"): m = November; break;
	case m2i("Dec"): m = December; break;
	default: return {};
	}
	return year(d(10) + 10 * d(9) + 100 * d(8) + 1000 * d(7)) / m / (d(5) + ((__DATE__[4] != ' ') ? 10 * d(4) : 0));
}

FString UFGUtils::GetGameVersion()
{
	constexpr std::chrono::year_month_day CompileDate = BuildCompileDate();
	
	// Format the year, month, and day into DDMMYYYY
	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << static_cast<unsigned>(CompileDate.day());
	oss << std::setw(2) << std::setfill('0') << static_cast<unsigned>(CompileDate.month());
	oss << std::setw(4) << std::setfill('0') << static_cast<int>(CompileDate.year());
	FString DateString(oss.str().c_str());
	
	static FString GameString = TEXT("SN1");
	return FString::Format(TEXT("{0} {1} {2}"), {
		GameString,
		DateString,
		LexToString(FApp::GetBuildConfiguration())
	});
}