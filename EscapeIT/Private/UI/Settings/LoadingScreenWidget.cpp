// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Settings/LoadingScreenWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CircularThrobber.h"
#include "Animation/WidgetAnimation.h"
#include "TimerManager.h"

ULoadingScreenWidget::ULoadingScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentProgress(0.0f)
	, TargetProgress(0.0f)
	, ProgressAnimationSpeed(2.0f)
	, bIsLoadingComplete(false)
	, CurrentStageIndex(0)
	, StageDisplayTime(2.0f)
{
	// Khởi tạo default loading messages
	LoadingStageMessages = {
		FText::FromString(TEXT("Initializing...")),
		FText::FromString(TEXT("Validating Settings...")),
		FText::FromString(TEXT("Applying Changes...")),
		FText::FromString(TEXT("Finalizing...")),
		FText::FromString(TEXT("Complete!"))
	};
}

void ULoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize UI
	CurrentProgress = 0.0f;
	TargetProgress = 0.0f;
	bIsLoadingComplete = false;
	CurrentStageIndex = 0;

	// Setup initial state
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(0.0f);
	}

	if (LoadingStatusText && LoadingStageMessages.Num() > 0)
	{
		LoadingStatusText->SetText(LoadingStageMessages[0]);
		CurrentStageText = LoadingStageMessages[0];
	}

	if (ProgressPercentageText)
	{
		ProgressPercentageText->SetText(FText::FromString(TEXT("0%")));
	}

	// Enable tick for smooth progress animation
	SetVisibility(ESlateVisibility::Collapsed);

	UE_LOG(LogTemp, Log, TEXT("LoadingScreenWidget: Constructed"));
}

void ULoadingScreenWidget::NativeDestruct()
{
	// Clear any active timers
	UWorld* World = GetWorld();
	if (World)
	{
		if (StageAdvanceTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(StageAdvanceTimerHandle);
		}
		if (FadeOutCompleteHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(FadeOutCompleteHandle);
		}
	}

	Super::NativeDestruct();
}

void ULoadingScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Smooth progress bar animation
	if (CurrentProgress != TargetProgress)
	{
		InterpolateProgress(InDeltaTime);
	}
}

// ===== PUBLIC FUNCTIONS =====

void ULoadingScreenWidget::Show()
{
	UE_LOG(LogTemp, Log, TEXT("LoadingScreenWidget: Showing loading screen"));

	SetVisibility(ESlateVisibility::Visible);

	// Play fade in animation
	if (FadeInAnimation)
	{
		PlayAnimation(FadeInAnimation);
	}

	// Play loading animations
	PlayLoadingAnimations();

	// Broadcast event
	OnLoadingScreenShown.Broadcast();

	// Reset state
	ResetLoadingScreen();
}

void ULoadingScreenWidget::Hide()
{
	UE_LOG(LogTemp, Log, TEXT("LoadingScreenWidget: Hiding loading screen"));

	// Stop loading animations
	StopLoadingAnimations();

	// Play fade out animation
	if (FadeOutAnimation)
	{
		PlayAnimation(FadeOutAnimation);

		// Wait for animation to complete before hiding
		UWorld* World = GetWorld();
		if (World)
		{
			float AnimDuration = FadeOutAnimation->GetEndTime();
			World->GetTimerManager().SetTimer(
				FadeOutCompleteHandle,
				[this]()
				{
					SetVisibility(ESlateVisibility::Collapsed);
					OnLoadingScreenHidden.Broadcast();
				},
				AnimDuration,
				false
			);
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
		OnLoadingScreenHidden.Broadcast();
	}
}

void ULoadingScreenWidget::UpdateProgress(float NewProgress)
{
	// Clamp progress to 0-1
	TargetProgress = FMath::Clamp(NewProgress, 0.0f, 1.0f);

	UE_LOG(LogTemp, Log, TEXT("LoadingScreenWidget: Progress updated to %.1f%%"),
		TargetProgress * 100.0f);
}

void ULoadingScreenWidget::UpdateStatusText(const FText& StatusText)
{
	CurrentStageText = StatusText;

	if (LoadingStatusText)
	{
		LoadingStatusText->SetText(StatusText);
	}

	UE_LOG(LogTemp, Log, TEXT("LoadingScreenWidget: Status updated - %s"),
		*StatusText.ToString());
}

void ULoadingScreenWidget::AdvanceToNextStage()
{
	if (CurrentStageIndex < LoadingStageMessages.Num() - 1)
	{
		CurrentStageIndex++;
		SetLoadingStage(CurrentStageIndex);
	}
}

void ULoadingScreenWidget::SetLoadingStage(int32 StageIndex)
{
	if (!LoadingStageMessages.IsValidIndex(StageIndex))
		return;

	CurrentStageIndex = StageIndex;
	UpdateStatusText(LoadingStageMessages[StageIndex]);

	// Auto-update progress based on stage
	float ProgressPerStage = 1.0f / FMath::Max(1, LoadingStageMessages.Num() - 1);
	UpdateProgress(StageIndex * ProgressPerStage);

	UE_LOG(LogTemp, Log, TEXT("LoadingScreenWidget: Advanced to stage %d"), StageIndex);
}

void ULoadingScreenWidget::CompleteLoading()
{
	UE_LOG(LogTemp, Log, TEXT("LoadingScreenWidget: Loading complete"));

	bIsLoadingComplete = true;

	// Set progress to 100%
	UpdateProgress(1.0f);

	// Set final stage
	if (LoadingStageMessages.Num() > 0)
	{
		CurrentStageIndex = LoadingStageMessages.Num() - 1;
		UpdateStatusText(LoadingStageMessages[CurrentStageIndex]);
	}

	// Broadcast completion
	OnLoadingComplete.Broadcast();

	// Auto-hide after a short delay
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			FadeOutCompleteHandle,
			this,
			&ULoadingScreenWidget::Hide,
			0.5f,
			false
		);
	}
}

void ULoadingScreenWidget::ResetLoadingScreen()
{
	CurrentProgress = 0.0f;
	TargetProgress = 0.0f;
	CurrentStageIndex = 0;
	bIsLoadingComplete = false;

	UpdateProgress(0.0f);

	if (LoadingStageMessages.Num() > 0)
	{
		UpdateStatusText(LoadingStageMessages[0]);
	}

	UpdateProgressBar();
	UpdatePercentageText();

	UE_LOG(LogTemp, Log, TEXT("LoadingScreenWidget: Reset"));
}

// ===== INTERNAL FUNCTIONS =====

void ULoadingScreenWidget::UpdateProgressBar()
{
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(CurrentProgress);
	}
}

void ULoadingScreenWidget::UpdatePercentageText()
{
	if (ProgressPercentageText)
	{
		int32 Percentage = FMath::RoundToInt(CurrentProgress * 100.0f);
		FText PercentText = FText::FromString(FString::Printf(TEXT("%d%%"), Percentage));
		ProgressPercentageText->SetText(PercentText);
	}
}

void ULoadingScreenWidget::PlayLoadingAnimations()
{
	// Play pulse animation on loop
	if (LoadingPulseAnimation)
	{
		PlayAnimation(LoadingPulseAnimation, 0.0f, 0); // Loop indefinitely
	}
}

void ULoadingScreenWidget::StopLoadingAnimations()
{
	// Stop pulse animation
	if (LoadingPulseAnimation)
	{
		StopAnimation(LoadingPulseAnimation);
	}
}

void ULoadingScreenWidget::InterpolateProgress(float DeltaTime)
{
	// Smooth interpolation
	CurrentProgress = FMath::FInterpTo(
		CurrentProgress,
		TargetProgress,
		DeltaTime,
		ProgressAnimationSpeed
	);

	// Update visuals
	UpdateProgressBar();
	UpdatePercentageText();

	// Check if we reached target
	if (FMath::IsNearlyEqual(CurrentProgress, TargetProgress, 0.001f))
	{
		CurrentProgress = TargetProgress;
	}
}