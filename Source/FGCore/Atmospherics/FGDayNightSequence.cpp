// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGDayNightSequence.h"

#include "LevelSequencePlayer.h"

AFGDayNightSequence::AFGDayNightSequence(const FObjectInitializer& Init)
	: Super(Init)
{
	PrimaryActorTick.bCanEverTick = true;
	
	SkySphereMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkySphereMeshComponent"));
	SkySphereMeshComponent->SetupAttachment(RootComponent);

	PlaybackSettings.bAutoPlay = true;
	PlaybackSettings.LoopCount.Value = INDEX_NONE; // Infinite Loop.
}

void AFGDayNightSequence::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if(!GetSequence()) // Invalid sequence, early out.
	{
		return;
	}

	PlaybackSettings.StartTime = NormalizedTimeToSequenceSecs(InitialTimeOfDay);

	// Playrate is the sequence length in seconds (2400 for example - 24 hours)
	// divided by the day length in minutes (10 for example - 10 minutes).
	PlaybackSettings.PlayRate = GetSequenceEndSecs() / (DayLengthMinutes * 60.0f);
	
	// Initialize player in editor.
	if(GetWorld()->IsEditorWorld() && !IsTemplate())
	{
		// Level sequence is already loaded. Initialize the player if it's not already initialized with this sequence
		if (LevelSequenceAsset != GetSequence() || GetSequencePlayer()->GetEvaluationTemplate().GetRunner() == nullptr)
		{
			GetSequencePlayer()->Initialize(LevelSequenceAsset, GetLevel(), CameraSettings);
		}

		if(bPlayInEditor) // Begin playback.
		{
			GetSequencePlayer()->Play();
		}
		else // Begin paused (Create spawnables and pause).
		{
			// @FIXME: Following code didn't work, actually breaks sequencer.
			
			// @HACK: Temporary hack to initialize player paused.
			//GetSequencePlayer()->Play();

			// Wait a frame, then pause.
			//GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
			//{
			//	GetSequencePlayer()->Pause();
			//}));
		}
	}
}

#if WITH_EDITOR
void AFGDayNightSequence::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(!PropertyChangedEvent.Property) // No property changed, early out.
	{
		return;
	}
	
	// Check if changed property is bPlayInEditor.
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGDayNightSequence, bPlayInEditor))
	{
		if(bPlayInEditor) // Begin playback.
		{
			GetSequencePlayer()->Play();
		}
		else // Pause playback.
		{
			GetSequencePlayer()->Pause();
		}
	}

	// Check if changed property is DayLengthMinutes.
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGDayNightSequence, DayLengthMinutes))
	{
		PlaybackSettings.PlayRate = GetSequenceEndSecs() / (DayLengthMinutes * 60.0f);
	}

	// Check if changed property is InitialTimeOfDay.
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGDayNightSequence, InitialTimeOfDay))
	{
		if(!bPlayInEditor) // Only scrub if we aren't playing.
		{
			PlaybackSettings.StartTime = NormalizedTimeToSequenceSecs(InitialTimeOfDay);

			// @FIXME: Works but slowly scrubs to the new time. We want to jump time.
			
			// Scrub to the new initial time.
			GetSequencePlayer()->PlayTo(
				FMovieSceneSequencePlaybackParams(
					NormalizedTimeToSequenceSecs(InitialTimeOfDay),
					EUpdatePositionMethod::Jump
				),
				FMovieSceneSequencePlayToParams()
			);
		}
	}
}
#endif

float AFGDayNightSequence::GetSequenceEndSecs()
{
	// Get end in frametime, then convert to seconds.
	UMovieScene*		MovieScene	= GetSequence()->GetMovieScene();
	const FFrameNumber	Frame		= MovieScene->GetPlaybackRange().Size<FFrameNumber>();
	const float			EndSeconds	= MovieScene->GetTickResolution().AsSeconds(Frame);
	return EndSeconds;
}

float AFGDayNightSequence::NormalizedTimeToSequenceSecs(float InTime)
{
	// Unnormalized time in seconds between start and end.
	return FMath::GetMappedRangeValueClamped(
		FVector2f(0.0f, 1.0f),
		FVector2f(0.0f, GetSequenceEndSecs()),
		InTime
	);
}
