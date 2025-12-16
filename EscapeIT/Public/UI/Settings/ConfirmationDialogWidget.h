// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConfirmationDialogWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UWidgetAnimation;
class UProgressBar;

// ===== DELEGATES =====

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmationAccepted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmationDeclined);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownWarning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, float, RemainingSeconds);

UCLASS()
class ESCAPEIT_API UConfirmationDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UConfirmationDialogWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ===== UI COMPONENTS =====

	/** Accept/Confirm button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	/** Decline/Cancel button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	/** Dialog title text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/** Dialog message text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;

	/** Countdown timer text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> CountdownText;

	/** Countdown progress bar */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> CountdownProgressBar;

	/** Background overlay */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> BackgroundOverlay;

	/** Confirm button text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> ConfirmButtonText;

	/** Cancel button text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> CancelButtonText;

	// ===== ANIMATIONS =====

	/** Dialog appear animation */
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ShowAnimation;

	/** Dialog hide animation */
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> HideAnimation;

	/** Countdown warning animation (when time is low) */
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> WarningAnimation;

	// ===== PROPERTIES =====

	/** Total countdown time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Confirmation")
	float TotalCountdownTime;

	/** Remaining time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Confirmation")
	float RemainingTime;

	/** Whether countdown is active */
	UPROPERTY(BlueprintReadOnly, Category = "Confirmation")
	bool bIsCountdownActive;

	/** Time threshold to show warning animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Confirmation")
	float WarningTimeThreshold;

	/** Whether warning animation is playing */
	UPROPERTY(BlueprintReadOnly, Category = "Confirmation")
	bool bIsWarningActive;

	// ===== CUSTOMIZATION =====

	/** Dialog title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Confirmation|Content")
	FText DialogTitle;

	/** Dialog message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Confirmation|Content")
	FText DialogMessage;

	/** Confirm button label */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Confirmation|Content")
	FText ConfirmButtonLabel;

	/** Cancel button label */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Confirmation|Content")
	FText CancelButtonLabel;

	/** Auto-decline on timeout */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Confirmation")
	bool bAutoDeclineOnTimeout;

	// ===== EVENTS =====

	/** Called when user confirms */
	UPROPERTY(BlueprintAssignable, Category = "Confirmation|Events")
	FOnConfirmationAccepted OnConfirmationAccepted;

	/** Called when user declines or timeout */
	UPROPERTY(BlueprintAssignable, Category = "Confirmation|Events")
	FOnConfirmationDeclined OnConfirmationDeclined;

	/** Called when countdown reaches threshold */
	UPROPERTY(BlueprintAssignable, Category = "Confirmation|Events")
	FOnCountdownWarning OnCountdownWarning;

	/** Called each second of countdown */
	UPROPERTY(BlueprintAssignable, Category = "Confirmation|Events")
	FOnCountdownTick OnCountdownTick;

	// ===== PUBLIC FUNCTIONS =====

	/** Show dialog with default countdown */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void ShowDialog();

	/** Show dialog with custom countdown time */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void ShowDialogWithCountdown(float CountdownSeconds);

	/** Show dialog with custom message */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void ShowDialogWithMessage(const FText& Title, const FText& Message, float CountdownSeconds);

	/** Hide dialog */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void HideDialog();

	/** Start countdown timer */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void StartCountdown();

	/** Stop countdown timer */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void StopCountdown();

	/** Reset countdown to initial value */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void ResetCountdown();

	/** Extend countdown time */
	UFUNCTION(BlueprintCallable, Category = "Confirmation Dialog")
	void ExtendCountdown(float AdditionalSeconds);

protected:
	// ===== BUTTON CALLBACKS =====

	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

private:
	// ===== INTERNAL FUNCTIONS =====

	/** Bind button events */
	void BindButtonEvents();

	/** Unbind button events */
	void UnbindButtonEvents();

	/** Update countdown display */
	void UpdateCountdownDisplay();

	/** Update progress bar */
	void UpdateProgressBar();

	/** Format time for display */
	FText FormatTime(float TimeInSeconds) const;

	/** Check if warning should be triggered */
	void CheckWarningThreshold();

	/** Handle countdown tick */
	void OnCountdownTick_Internal();

	/** Handle countdown timeout */
	void OnCountdownTimeout();

	// ===== TIMER HANDLE =====

	/** Timer for countdown */
	FTimerHandle CountdownTimerHandle;
};