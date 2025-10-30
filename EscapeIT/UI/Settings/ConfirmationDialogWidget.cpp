// Fill out your copyright notice in the Description page of Project Settings.

#include "ConfirmationDialogWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Animation/WidgetAnimation.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UConfirmationDialogWidget::UConfirmationDialogWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TotalCountdownTime(15.0f)
	, RemainingTime(15.0f)
	, bIsCountdownActive(false)
	, WarningTimeThreshold(5.0f)
	, bIsWarningActive(false)
	, bAutoDeclineOnTimeout(true)
{
	// Default texts
	DialogTitle = FText::FromString(TEXT("Keep These Settings?"));
	DialogMessage = FText::FromString(TEXT("Your display settings have been changed. Do you want to keep these settings?"));
	ConfirmButtonLabel = FText::FromString(TEXT("Keep Changes"));
	CancelButtonLabel = FText::FromString(TEXT("Revert"));
}

void UConfirmationDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button events
	BindButtonEvents();

	// Set initial texts
	if (TitleText)
	{
		TitleText->SetText(DialogTitle);
	}

	if (MessageText)
	{
		MessageText->SetText(DialogMessage);
	}

	if (ConfirmButtonText)
	{
		ConfirmButtonText->SetText(ConfirmButtonLabel);
	}

	if (CancelButtonText)
	{
		CancelButtonText->SetText(CancelButtonLabel);
	}

	// Initialize countdown
	RemainingTime = TotalCountdownTime;
	bIsCountdownActive = false;
	bIsWarningActive = false;

	UpdateCountdownDisplay();
	UpdateProgressBar();

	// Initially hidden
	SetVisibility(ESlateVisibility::Collapsed);

	UE_LOG(LogTemp, Log, TEXT("ConfirmationDialogWidget: Constructed"));
}

void UConfirmationDialogWidget::NativeDestruct()
{
	// Unbind events
	UnbindButtonEvents();

	// Stop countdown
	StopCountdown();

	Super::NativeDestruct();
}

void UConfirmationDialogWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update visuals during countdown
	if (bIsCountdownActive)
	{
		UpdateProgressBar();
	}
}

// ===== PUBLIC FUNCTIONS =====

void UConfirmationDialogWidget::ShowDialog()
{
	ShowDialogWithCountdown(TotalCountdownTime);
}

void UConfirmationDialogWidget::ShowDialogWithCountdown(float CountdownSeconds)
{
	UE_LOG(LogTemp, Log, TEXT("ConfirmationDialogWidget: Showing dialog with %.1fs countdown"),
		CountdownSeconds);

	TotalCountdownTime = CountdownSeconds;
	RemainingTime = CountdownSeconds;

	SetVisibility(ESlateVisibility::Visible);

	// Play show animation
	if (ShowAnimation)
	{
		PlayAnimation(ShowAnimation);
	}

	// Start countdown
	StartCountdown();

	// Set focus to confirm button
	if (ConfirmButton)
	{
		ConfirmButton->SetKeyboardFocus();
	}
}

void UConfirmationDialogWidget::ShowDialogWithMessage(const FText& Title, const FText& Message, float CountdownSeconds)
{
	DialogTitle = Title;
	DialogMessage = Message;

	if (TitleText)
	{
		TitleText->SetText(Title);
	}

	if (MessageText)
	{
		MessageText->SetText(Message);
	}

	ShowDialogWithCountdown(CountdownSeconds);
}

void UConfirmationDialogWidget::HideDialog()
{
	UE_LOG(LogTemp, Log, TEXT("ConfirmationDialogWidget: Hiding dialog"));

	StopCountdown();

	// Play hide animation
	if (HideAnimation)
	{
		PlayAnimation(HideAnimation);

		// Wait for animation then hide
		UWorld* World = GetWorld();
		if (World)
		{
			float AnimDuration = HideAnimation->GetEndTime();
			FTimerHandle HideTimerHandle;
			World->GetTimerManager().SetTimer(
				HideTimerHandle,
				[this]()
				{
					SetVisibility(ESlateVisibility::Collapsed);
				},
				AnimDuration,
				false
			);
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	// Stop warning animation
	if (WarningAnimation && bIsWarningActive)
	{
		StopAnimation(WarningAnimation);
		bIsWarningActive = false;
	}
}

void UConfirmationDialogWidget::StartCountdown()
{
	if (bIsCountdownActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfirmationDialogWidget: Countdown already active"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ConfirmationDialogWidget: Starting countdown"));

	bIsCountdownActive = true;
	bIsWarningActive = false;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&UConfirmationDialogWidget::OnCountdownTick_Internal,
			1.0f,
			true
		);
	}
}

void UConfirmationDialogWidget::StopCountdown()
{
	if (!bIsCountdownActive)
		return;

	UE_LOG(LogTemp, Log, TEXT("ConfirmationDialogWidget: Stopping countdown"));

	bIsCountdownActive = false;

	UWorld* World = GetWorld();
	if (World && CountdownTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
		CountdownTimerHandle.Invalidate();
	}
}

void UConfirmationDialogWidget::ResetCountdown()
{
	StopCountdown();
	RemainingTime = TotalCountdownTime;
	bIsWarningActive = false;
	UpdateCountdownDisplay();
	UpdateProgressBar();
}

void UConfirmationDialogWidget::ExtendCountdown(float AdditionalSeconds)
{
	RemainingTime += AdditionalSeconds;
	TotalCountdownTime += AdditionalSeconds;

	UE_LOG(LogTemp, Log, TEXT("ConfirmationDialogWidget: Extended countdown by %.1fs (new total: %.1fs)"),
		AdditionalSeconds, RemainingTime);

	UpdateCountdownDisplay();
	UpdateProgressBar();
}

// ===== BUTTON CALLBACKS =====

void UConfirmationDialogWidget::OnConfirmButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("ConfirmationDialogWidget: User confirmed"));

	StopCountdown();

	// Broadcast acceptance
	OnConfirmationAccepted.Broadcast();

	// Hide dialog
	HideDialog();
}

void UConfirmationDialogWidget::OnCancelButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("ConfirmationDialogWidget: User cancelled"));

	StopCountdown();

	// Broadcast decline
	OnConfirmationDeclined.Broadcast();

	// Hide dialog
	HideDialog();
}

// ===== INTERNAL FUNCTIONS =====

void UConfirmationDialogWidget::BindButtonEvents()
{
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UConfirmationDialogWidget::OnConfirmButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UConfirmationDialogWidget::OnCancelButtonClicked);
	}
}

void UConfirmationDialogWidget::UnbindButtonEvents()
{
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UConfirmationDialogWidget::OnConfirmButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.RemoveDynamic(this, &UConfirmationDialogWidget::OnCancelButtonClicked);
	}
}

void UConfirmationDialogWidget::UpdateCountdownDisplay()
{
	if (CountdownText)
	{
		FText TimeText = FormatTime(RemainingTime);
		CountdownText->SetText(TimeText);
	}
}

void UConfirmationDialogWidget::UpdateProgressBar()
{
	if (CountdownProgressBar)
	{
		float ProgressPercent = RemainingTime / FMath::Max(TotalCountdownTime, 1.0f);
		CountdownProgressBar->SetPercent(ProgressPercent);
	}
}

FText UConfirmationDialogWidget::FormatTime(float TimeInSeconds) const
{
	int32 Seconds = FMath::CeilToInt(TimeInSeconds);

	if (Seconds > 60)
	{
		int32 Minutes = Seconds / 60;
		int32 RemainingSecs = Seconds % 60;
		return FText::FromString(FString::Printf(TEXT("Reverting in %d:%02d"), Minutes, RemainingSecs));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("Reverting in %d seconds"), Seconds));
	}
}

void UConfirmationDialogWidget::CheckWarningThreshold()
{
	if (!bIsWarningActive && RemainingTime <= WarningTimeThreshold)
	{
		bIsWarningActive = true;

		// Play warning animation
		if (WarningAnimation)
		{
			PlayAnimation(WarningAnimation, 0.0f, 0); // Loop
		}

		// Broadcast warning event
		OnCountdownWarning.Broadcast();

		UE_LOG(LogTemp, Warning, TEXT("ConfirmationDialogWidget: Countdown warning threshold reached"));
	}
}

void UConfirmationDialogWidget::OnCountdownTick_Internal()
{
	if (!bIsCountdownActive)
		return;

	RemainingTime -= 1.0f;

	// Update display
	UpdateCountdownDisplay();

	// Broadcast tick event
	OnCountdownTick.Broadcast(RemainingTime);

	// Check warning threshold
	CheckWarningThreshold();

	// Check timeout
	if (RemainingTime <= 0.0f)
	{
		OnCountdownTimeout();
	}
}

void UConfirmationDialogWidget::OnCountdownTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("ConfirmationDialogWidget: Countdown timeout"));

	StopCountdown();

	if (bAutoDeclineOnTimeout)
	{
		// Auto-decline
		OnConfirmationDeclined.Broadcast();
		HideDialog();
	}
	else
	{
		// Just disable the confirm button
		if (ConfirmButton)
		{
			ConfirmButton->SetIsEnabled(false);
		}

		// Still allow manual cancel
		UpdateCountdownDisplay();
	}
}