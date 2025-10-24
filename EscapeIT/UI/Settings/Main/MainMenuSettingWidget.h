// MainMenuSettingWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/EscapeITEnums.h"
#include "MainMenuSettingWidget.generated.h"

// Forward declarations
class UWidgetSwitcher;
class UButton;
class UEditableTextBox;
class UTextBlock;
class UCommonTextBlock;
class UImage;
class UGameplayWidget;
class UGraphicWidget;
class UAudioWidget;
class UControlWidget;
class UAccessibilityWidget;
class USettingsSubsystem;
class USoundBase;
class UNotificationWidget;

USTRUCT(BlueprintType)
struct FSettingChange
{
	GENERATED_BODY()

	UPROPERTY()
	FString SettingName;

	UPROPERTY()
	FString OldValue;

	UPROPERTY()
	FString NewValue;

	UPROPERTY()
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

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnBackClickedSignature OnBackClicked;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingsAppliedSignature OnSettingsApplied;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnCategoryChangedSignature OnCategoryChanged;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// ===== WIDGET REFERENCES =====

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

	UPROPERTY(meta = (BindWidget))
	UButton* UndoButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AutoDetectButton;

	// Text Elements
	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* CategoryTitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UnsavedChangesText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PerformanceEstimateText;

	// Other UI Elements
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* SearchBox;

	UPROPERTY(meta = (BindWidget))
	UImage* UnsavedIndicator;

	UPROPERTY(meta = (BindWidget))
	UImage* CategoryIcon;

	// Category Widgets
	UPROPERTY(meta = (BindWidget))
	UGameplayWidget* GameplayWidget;

	UPROPERTY(meta = (BindWidget))
	UGraphicWidget* GraphicWidget;

	UPROPERTY(meta = (BindWidget))
	UAudioWidget* AudioWidget;

	UPROPERTY(meta = (BindWidget))
	UControlWidget* ControlWidget;

	UPROPERTY(meta = (BindWidget))
	UAccessibilityWidget* AccessibilityWidget;

	// Optional Notification Widget
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> NotificationWidget;

	// ===== ANIMATIONS =====

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CategorySwitchAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* UnsavedWarningAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ApplySuccessAnimation;

	// ===== SOUNDS =====

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* ButtonClickSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* CategorySwitchSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* ApplySound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* ErrorSound;

	// ===== BUTTON CALLBACKS =====

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

	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);

	// ===== INTERNAL FUNCTIONS =====

	void BindButtonEvents();
	void UnbindButtonEvents();
	void SwitchToCategory(ESettingsCategory Category);
	void UpdateCategoryButtons(ESettingsCategory ActiveCategory);
	void UpdateCategoryTitle(ESettingsCategory Category);
	void UpdateCategoryIcon(ESettingsCategory Category);
	void SetButtonActive(UButton* Button, bool bActive);
	FText GetCategoryName(ESettingsCategory Category) const;

	// Settings Management
	void CollectSettingsFromWidgets();
	void AsyncApplySettings();
	void OnSettingsApplied_Internal(bool bSuccess);
	bool ValidateAllSettings();
	void RefreshAllWidgets();

	// Change Tracking
	void MarkSettingsChanged();
	void ClearUnsavedChanges();
	void CheckForUnsavedChanges();
	void UpdateUnsavedIndicator();
	void TrackSettingChange(const FString& SettingName, const FString& OldValue, const FString& NewValue);
	void UndoLastChange();
	void UpdateUndoButton();
	bool CanUndo() const { return RecentChanges.Num() > 0; }

	// Dialogs
	void ShowUnsavedChangesDialog();
	void OnUnsavedChangesDialogResponse(bool bSaveChanges);
	void ShowValidationErrors(const TArray<FString>& Errors);
	void ShowSuccessNotification(const FString& Message);
	void ShowErrorNotification(const FString& Message);

	// Navigation
	void NavigateToNextCategory();
	void NavigateToPreviousCategory();

	// Search & Filter
	void FilterSettingsBySearch(const FString& SearchText);
	void ClearSearch();

	// Graphics Presets
	void ApplyGraphicsPreset(EE_GraphicsQuality Preset);
	void AutoDetectOptimalSettings();

	// Performance
	void UpdatePerformanceEstimate();
	FText GetPerformanceImpactText() const;
	void ValidateSettingsForHardware();

	// Animations
	void PlayCategorySwitchAnimation();
	void PlayUnsavedWarningAnimation();
	void PlayApplySuccessAnimation();

	// Auto-save
	void StartAutoSaveTimer();
	void StopAutoSaveTimer();
	void OnAutoSaveTimerElapsed();

	// Audio
	void PlayUISound(const FString& SoundName);

	// Accessibility
	void AnnounceCurrentCategory();
	void AnnounceSettingChange(const FString& SettingName, const FString& NewValue);

	// Tooltips & Help
	void ShowSettingsHelp(ESettingsCategory Category);
	FText GetSettingTooltip(const FString& SettingName) const;

	// Profiles
	void SaveCustomProfile(const FString& ProfileName);
	void LoadCustomProfile(const FString& ProfileName);

	// ===== PROPERTIES =====

	UPROPERTY()
	USettingsSubsystem* SettingsSubsystem;

	UPROPERTY()
	int32 CurrentCategoryIndex;

	UPROPERTY()
	bool bHasUnsavedChanges;

	UPROPERTY()
	bool bIsApplyingSettings;

	UPROPERTY()
	FString CurrentSearchText;

	UPROPERTY()
	TArray<FSettingChange> RecentChanges;

	UPROPERTY()
	TArray<FString> ValidationErrors;

	// Pending settings to apply
	UPROPERTY()
	FS_AllSettings PendingSettings;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	int32 MaxRecentChanges = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float AutoSaveDelay = 30.0f;

	FTimerHandle AutoSaveTimerHandle;

public:
	// Public API
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ForceApplySettings();
};