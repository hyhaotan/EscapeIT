// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuSettingWidget.generated.h"

// Forward Declarations
class UWidgetSwitcher;
class UButton;
class UCommonTextBlock;
class UGameplayWidget;
class UGraphicWidget;
class UAudioWidget;
class UControlWidget;
class UAccessibilityWidget;
class USettingsSubsystem;
class UEditableTextBox;
class UTextBlock;
class UImage;
class UWidgetAnimation;
class UConfirmationDialogWidget;

// Struct to track setting changes
USTRUCT(BlueprintType)
struct FSettingChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SettingName;

	UPROPERTY(BlueprintReadWrite)
	FString OldValue;

	UPROPERTY(BlueprintReadWrite)
	FString NewValue;

	UPROPERTY(BlueprintReadWrite)
	FDateTime ChangeTime;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackClickedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsAppliedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategoryChangedSignature, ESettingsCategory, NewCategory);

UCLASS()
class ESCAPEIT_API UMainMenuSettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuSettingWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
	FOnBackClickedSignature OnBackClicked;

	UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
	FOnSettingsAppliedSignature OnSettingsApplied;

	UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
	FOnCategoryChangedSignature OnCategoryChanged;

protected:
	// ===== WIDGET COMPONENTS =====

	// Main Switcher
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* SettingsSwitcher;

	// Category Buttons
	UPROPERTY(meta = (BindWidget))
	UButton* GameplayButton;

	UPROPERTY(meta = (BindWidget))
	UButton* GraphicsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AudioButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ControlsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AccessibilityButton;

	// Action Buttons
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ApplyAllButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetAllButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* UndoButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* AutoDetectButton;

	// Text Elements
	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* CategoryTitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* UnsavedChangesText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* PerformanceEstimateText;

	// Search Box (Optional)
	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* SearchBox;

	// Visual Elements
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* CategoryIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* UnsavedIndicator;

	// Category Widgets
	UPROPERTY(meta = (BindWidgetOptional))
	UGameplayWidget* GameplayWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	UGraphicWidget* GraphicWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	UAudioWidget* AudioWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	UControlWidget* ControlWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	UAccessibilityWidget* AccessibilityWidget;

	// Animations (Optional - bind in Blueprint)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* CategorySwitchAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* UnsavedWarningAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* ApplySuccessAnimation;

	// ===== STATE VARIABLES =====

	UPROPERTY()
	USettingsSubsystem* SettingsSubsystem;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	int32 CurrentCategoryIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	bool bHasUnsavedChanges;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	FString CurrentSearchText;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	TArray<FSettingChange> RecentChanges;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	int32 MaxRecentChanges = 10;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	bool bIsApplyingSettings;

	// Timer for auto-save
	FTimerHandle AutoSaveTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float AutoSaveDelay = 3.0f;

	// ===== BUTTON BINDING =====

	void BindButtonEvents();
	void UnbindButtonEvents();

	// Category Button Callbacks
	UFUNCTION()
	void OnGameplayButtonClicked();

	UFUNCTION()
	void OnGraphicsButtonClicked();

	UFUNCTION()
	void OnAudioButtonClicked();

	UFUNCTION()
	void OnControlsButtonClicked();

	UFUNCTION()
	void OnAccessibilityButtonClicked();

	// Action Button Callbacks
	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnApplyAllButtonClicked();

	UFUNCTION()
	void OnResetAllButtonClicked();

	UFUNCTION()
	void OnUndoButtonClicked();

	UFUNCTION()
	void OnAutoDetectButtonClicked();

	// Search Callback
	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);

	// ===== CATEGORY MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SwitchToCategory(ESettingsCategory Category);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void NavigateToNextCategory();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void NavigateToPreviousCategory();

	void UpdateCategoryButtons(ESettingsCategory ActiveCategory);
	void UpdateCategoryTitle(ESettingsCategory Category);
	void UpdateCategoryIcon(ESettingsCategory Category);

	UFUNCTION(BlueprintPure, Category = "Settings")
	FText GetCategoryName(ESettingsCategory Category) const;

	void SetButtonActive(UButton* Button, bool bActive);

	// ===== UNSAVED CHANGES TRACKING =====

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void MarkSettingsChanged();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ClearUnsavedChanges();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void CheckForUnsavedChanges();

	void ShowUnsavedChangesDialog();
	void OnUnsavedChangesDialogResponse(bool bSaveChanges);

	void UpdateUnsavedIndicator();

	// ===== CHANGE TRACKING =====

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void TrackSettingChange(const FString& SettingName, const FString& OldValue, const FString& NewValue);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void UndoLastChange();

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool CanUndo() const { return RecentChanges.Num() > 0; }

	void UpdateUndoButton();

	// ===== GRAPHICS PRESETS =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void ApplyGraphicsPreset(EE_GraphicsQuality Preset);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void AutoDetectOptimalSettings();

	// ===== SEARCH & FILTER =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Search")
	void FilterSettingsBySearch(const FString& SearchText);

	UFUNCTION(BlueprintCallable, Category = "Settings|Search")
	void ClearSearch();

	// ===== VALIDATION =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Validation")
	bool ValidateAllSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings|Validation")
	void ValidateSettingsForHardware();

	void ShowValidationErrors(const TArray<FString>& Errors);

	// ===== PERFORMANCE ESTIMATION =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Performance")
	void UpdatePerformanceEstimate();

	UFUNCTION(BlueprintPure, Category = "Settings|Performance")
	FText GetPerformanceImpactText() const;

	// ===== ANIMATIONS =====

	void PlayCategorySwitchAnimation();
	void PlayUnsavedWarningAnimation();
	void PlayApplySuccessAnimation();

	// ===== ASYNC OPERATIONS =====

	void AsyncApplySettings();
	void OnSettingsApplied_Internal(bool bSuccess);

	// ===== AUTO-SAVE =====

	void StartAutoSaveTimer();
	void StopAutoSaveTimer();
	void OnAutoSaveTimerElapsed();

	// ===== TOOLTIPS & HELP =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Help")
	void ShowSettingsHelp(ESettingsCategory Category);

	UFUNCTION(BlueprintCallable, Category = "Settings|Help")
	FText GetSettingTooltip(const FString& SettingName) const;

	// ===== PROFILES (Optional for future) =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Profiles")
	void SaveCustomProfile(const FString& ProfileName);

	UFUNCTION(BlueprintCallable, Category = "Settings|Profiles")
	void LoadCustomProfile(const FString& ProfileName);

	// ===== SOUND EFFECTS =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void PlayUISound(const FString& SoundName);

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	class USoundBase* ButtonClickSound;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	class USoundBase* CategorySwitchSound;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	class USoundBase* ApplySound;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	class USoundBase* ErrorSound;

	// ===== ACCESSIBILITY =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void AnnounceCurrentCategory();

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void AnnounceSettingChange(const FString& SettingName, const FString& NewValue);

public:
	// Public Blueprint-callable functions

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RefreshAllWidgets();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ForceApplySettings();

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool HasUnsavedChanges() const { return bHasUnsavedChanges; }

	UFUNCTION(BlueprintPure, Category = "Settings")
	ESettingsCategory GetCurrentCategory() const
	{
		return static_cast<ESettingsCategory>(CurrentCategoryIndex);
	}
};