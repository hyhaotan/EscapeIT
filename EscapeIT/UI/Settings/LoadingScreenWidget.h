// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UWidgetAnimation;
class UCircularThrobber;

// ===== DELEGATES =====

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingScreenShown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingScreenHidden);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingComplete);

UCLASS()
class ESCAPEIT_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	ULoadingScreenWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ===== UI COMPONENTS =====

	/** Progress bar showing loading progress */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingProgressBar;

	/** Text showing loading status */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> LoadingStatusText;

	/** Text showing loading percentage */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> ProgressPercentageText;

	/** Background blur/overlay */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> BackgroundOverlay;

	/** Loading spinner/throbber */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UCircularThrobber> LoadingThrobber;

	// ===== ANIMATIONS =====

	/** Fade in animation */
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeInAnimation;

	/** Fade out animation */
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeOutAnimation;

	/** Loading pulse animation */
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> LoadingPulseAnimation;

	// ===== PROPERTIES =====

	/** Current loading progress (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	float CurrentProgress;

	/** Target progress for smooth interpolation */
	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	float TargetProgress;

	/** Speed of progress bar animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	float ProgressAnimationSpeed;

	/** Current loading stage text */
	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	FText CurrentStageText;

	/** Whether loading is complete */
	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	bool bIsLoadingComplete;

	// ===== LOADING STAGES =====

	/** Predefined loading stage messages */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Loading|Stages")
	TArray<FText> LoadingStageMessages;

	/** Current stage index */
	UPROPERTY(BlueprintReadOnly, Category = "Loading|Stages")
	int32 CurrentStageIndex;

	/** Time to show each stage (if auto-progressing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading|Stages")
	float StageDisplayTime;

	// ===== PUBLIC FUNCTIONS =====

	/** Show the loading screen with fade in */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void Show();

	/** Hide the loading screen with fade out */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void Hide();

	/** Update loading progress (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void UpdateProgress(float NewProgress);

	/** Update loading status text */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void UpdateStatusText(const FText& StatusText);

	/** Set loading to next stage */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void AdvanceToNextStage();

	/** Set specific loading stage */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void SetLoadingStage(int32 StageIndex);

	/** Complete loading and hide */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void CompleteLoading();

	/** Reset loading screen state */
	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	void ResetLoadingScreen();

	// ===== EVENTS =====

	/** Called when loading screen is shown */
	UPROPERTY(BlueprintAssignable, Category = "Loading Screen|Events")
	FOnLoadingScreenShown OnLoadingScreenShown;

	/** Called when loading screen is hidden */
	UPROPERTY(BlueprintAssignable, Category = "Loading Screen|Events")
	FOnLoadingScreenHidden OnLoadingScreenHidden;

	/** Called when loading is complete */
	UPROPERTY(BlueprintAssignable, Category = "Loading Screen|Events")
	FOnLoadingComplete OnLoadingComplete;

private:
	// ===== INTERNAL FUNCTIONS =====

	/** Update progress bar visuals */
	void UpdateProgressBar();

	/** Update percentage text */
	void UpdatePercentageText();

	/** Play loading animations */
	void PlayLoadingAnimations();

	/** Stop loading animations */
	void StopLoadingAnimations();

	/** Smooth progress interpolation */
	void InterpolateProgress(float DeltaTime);

	// ===== TIMER HANDLES =====

	/** Handle for auto-stage advancement */
	FTimerHandle StageAdvanceTimerHandle;

	/** Handle for fade out completion */
	FTimerHandle FadeOutCompleteHandle;
};