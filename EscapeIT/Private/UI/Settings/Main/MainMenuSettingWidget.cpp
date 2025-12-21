// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Settings/Main/MainMenuSettingWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "CommonTextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "UI/Settings/Tab/GameplayWidget.h"
#include "UI/Settings/Tab/GraphicWidget.h"
#include "UI/Settings/Tab/AudioWidget.h"
#include "UI/Settings/Tab/ControlWidget.h"
#include "UI/Settings/Tab/AccessibilityWidget.h"
#include "Settings/Core/SettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "GameFramework/GameUserSettings.h"
#include "UI/NotificationWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Settings/LoadingScreenWidget.h"
#include "UI/Settings/ConfirmationDialogWidget.h"

UMainMenuSettingWidget::UMainMenuSettingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentCategoryIndex(0)
	, bHasUnsavedChanges(false)
	, bIsApplyingSettings(false)
	, AutoSaveDelay(5.0f)
	, MaxRecentChanges(10)
	, RemainingConfirmationTime(0.0f)
{
}

void UMainMenuSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get Settings Subsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
	}

	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Failed to get SettingsSubsystem"));
		return;
	}

	// Bind all button events
	BindButtonEvents();

	// Bind subsystem delegates
	BindSubsystemDelegates();

	// Bind child widget events
	BindChildWidgetEvents();

	// Initialize UI state
	bHasUnsavedChanges = false;
	UpdateUnsavedIndicator();
	UpdateUndoButton();

	// Start with Gameplay category
	SwitchToCategory(ESettingsCategory::Gameplay);

	// Setup search if available
	if (SearchBox)
	{
		SearchBox->SetHintText(FText::FromString(TEXT("Search settings...")));
	}


	if (WidgetManager)
	{
		InitializeTabWidgets();
	}

	// Refresh all widgets to sync with current settings
	RefreshAllWidgets();

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Initialized successfully"));
}

void UMainMenuSettingWidget::NativeDestruct()
{
	// Stop auto-save timer
	StopAutoSaveTimer();

	// Stop confirmation timer if active
	UWorld* World = GetWorld();
	if (World && ConfirmationCountdownHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(ConfirmationCountdownHandle);
	}

	// Unbind subsystem delegates
	UnbindSubsystemDelegates();

	// Unbind child widget events
	UnbindChildWidgetEvents();

	// Unbind all button events
	UnbindButtonEvents();

	// Clean up active widgets
	if (ActiveConfirmationDialog)
	{
		ActiveConfirmationDialog->RemoveFromParent();
		ActiveConfirmationDialog = nullptr;
	}

	if (ActiveLoadingScreen)
	{
		ActiveLoadingScreen->RemoveFromParent();
		ActiveLoadingScreen = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Destroyed"));

	Super::NativeDestruct();
}

void UMainMenuSettingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update performance estimate periodically
	static float UpdateTimer = 0.0f;
	UpdateTimer += InDeltaTime;
	if (UpdateTimer >= 1.0f)
	{
		UpdatePerformanceEstimate();
		UpdateTimer = 0.0f;
	}
}

FReply UMainMenuSettingWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FKey Key = InKeyEvent.GetKey();

	// Q or Gamepad Left Shoulder - Previous Category
	if (Key == EKeys::Q || Key == EKeys::Gamepad_LeftShoulder)
	{
		NavigateToPreviousCategory();
		return FReply::Handled();
	}

	// E or Gamepad Right Shoulder - Next Category
	if (Key == EKeys::E || Key == EKeys::Gamepad_RightShoulder)
	{
		NavigateToNextCategory();
		return FReply::Handled();
	}

	// Escape - Back
	if (Key == EKeys::Escape)
	{
		OnBackButtonClicked();
		return FReply::Handled();
	}

	// Ctrl+Z - Undo
	if (Key == EKeys::Z && InKeyEvent.IsControlDown())
	{
		OnUndoButtonClicked();
		return FReply::Handled();
	}

	// Ctrl+S - Apply
	if (Key == EKeys::S && InKeyEvent.IsControlDown())
	{
		OnApplyAllButtonClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ===== BUTTON BINDING =====

void UMainMenuSettingWidget::BindButtonEvents()
{
	if (GameplayButton)
		GameplayButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnGameplayButtonClicked);

	if (GraphicsButton)
		GraphicsButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnGraphicsButtonClicked);

	if (AudioButton)
		AudioButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnAudioButtonClicked);

	if (ControlsButton)
		ControlsButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnControlsButtonClicked);

	if (AccessibilityButton)
		AccessibilityButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnAccessibilityButtonClicked);

	if (BackButton)
		BackButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnBackButtonClicked);

	if (ApplyAllButton)
		ApplyAllButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnApplyAllButtonClicked);

	if (ResetAllButton)
		ResetAllButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnResetAllButtonClicked);

	if (UndoButton)
		UndoButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnUndoButtonClicked);

	if (AutoDetectButton)
		AutoDetectButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnAutoDetectButtonClicked);

	if (SearchBox)
		SearchBox->OnTextChanged.AddDynamic(this, &UMainMenuSettingWidget::OnSearchTextChanged);
}

void UMainMenuSettingWidget::UnbindButtonEvents()
{
	if (GameplayButton)
		GameplayButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnGameplayButtonClicked);

	if (GraphicsButton)
		GraphicsButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnGraphicsButtonClicked);

	if (AudioButton)
		AudioButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnAudioButtonClicked);

	if (ControlsButton)
		ControlsButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnControlsButtonClicked);

	if (AccessibilityButton)
		AccessibilityButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnAccessibilityButtonClicked);

	if (BackButton)
		BackButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnBackButtonClicked);

	if (ApplyAllButton)
		ApplyAllButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnApplyAllButtonClicked);

	if (ResetAllButton)
		ResetAllButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnResetAllButtonClicked);

	if (UndoButton)
		UndoButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnUndoButtonClicked);

	if (AutoDetectButton)
		AutoDetectButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnAutoDetectButtonClicked);

	if (SearchBox)
		SearchBox->OnTextChanged.RemoveDynamic(this, &UMainMenuSettingWidget::OnSearchTextChanged);
}

// ===== CATEGORY BUTTON CALLBACKS =====

void UMainMenuSettingWidget::OnGameplayButtonClicked()
{
	PlayUISound("ButtonClick");
	SwitchToCategory(ESettingsCategory::Gameplay);
}

void UMainMenuSettingWidget::OnGraphicsButtonClicked()
{
	PlayUISound("ButtonClick");
	SwitchToCategory(ESettingsCategory::Graphics);
}

void UMainMenuSettingWidget::OnAudioButtonClicked()
{
	PlayUISound("ButtonClick");
	SwitchToCategory(ESettingsCategory::Audio);
}

void UMainMenuSettingWidget::OnControlsButtonClicked()
{
	PlayUISound("ButtonClick");
	SwitchToCategory(ESettingsCategory::Controls);
}

void UMainMenuSettingWidget::OnAccessibilityButtonClicked()
{
	PlayUISound("ButtonClick");
	SwitchToCategory(ESettingsCategory::Accessibility);
}

// ===== MAIN BUTTON CALLBACKS =====

void UMainMenuSettingWidget::OnBackButtonClicked()
{
	if (bHasUnsavedChanges)
	{
		ShowUnsavedChangesDialog();
		return;
	}

	CloseSettingsMenu(true);
}

void UMainMenuSettingWidget::OnApplyAllButtonClicked()
{
	if (!SettingsSubsystem || bIsApplyingSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Cannot apply - subsystem null or already applying"));
		return;
	}

	PlayUISound("Apply");

	// Collect settings from all widgets
	CollectSettingsFromWidgets();

	// Validate before applying
	if (!ValidateAllSettings())
	{
		ShowValidationErrors(ValidationErrors);
		PlayUISound("Error");
		return;
	}

	// Show loading screen
	bIsApplyingSettings = true;
	ShowLoadingScreen();

	// Delay slightly to allow loading screen to render
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerHandle DelayHandle;
		World->GetTimerManager().SetTimer(DelayHandle, [this]()
			{
				AsyncApplySettings();
			}, 0.1f, false);
	}
	else
	{
		// Fallback if no world
		AsyncApplySettings();
	}
}

void UMainMenuSettingWidget::OnResetAllButtonClicked()
{
	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Cannot reset - SettingsSubsystem is null"));
		return;
	}

	PlayUISound("ButtonClick");

	// Reset all settings to default
	SettingsSubsystem->ResetSettingsToDefault();

	// Refresh all widgets
	RefreshAllWidgets();

	// Clear unsaved changes
	ClearUnsavedChanges();
	RecentChanges.Empty();
	UpdateUndoButton();

	ShowSuccessNotification(TEXT("All settings reset to default"));

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: All settings reset to default"));
}

void UMainMenuSettingWidget::OnUndoButtonClicked()
{
	if (CanUndo())
	{
		PlayUISound("ButtonClick");
		UndoLastChange();
	}
}

void UMainMenuSettingWidget::OnAutoDetectButtonClicked()
{
	PlayUISound("ButtonClick");
	AutoDetectOptimalSettings();
}

void UMainMenuSettingWidget::OnSearchTextChanged(const FText& Text)
{
	CurrentSearchText = Text.ToString();
	FilterSettingsBySearch(CurrentSearchText);
}

// ===== CATEGORY SWITCHING =====

void UMainMenuSettingWidget::SwitchToCategory(ESettingsCategory Category)
{
	if (!SettingsSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: SettingsSwitcher is null"));
		return;
	}

	// Play switch animation
	PlayCategorySwitchAnimation();

	// Play category switch sound
	PlayUISound("CategorySwitch");

	// Switch to the appropriate widget (enum starts at 1, so subtract 1)
	int32 CategoryIndex = static_cast<int32>(Category) - 1;

	// Ensure valid index
	if (CategoryIndex >= 0 && CategoryIndex < SettingsSwitcher->GetNumWidgets())
	{
		SettingsSwitcher->SetActiveWidgetIndex(CategoryIndex);
		CurrentCategoryIndex = CategoryIndex;

		// Update UI
		UpdateCategoryButtons(Category);
		UpdateCategoryTitle(Category);
		UpdateCategoryIcon(Category);

		// Announce for accessibility
		AnnounceCurrentCategory();

		// Broadcast event
		OnCategoryChanged.Broadcast(Category);

		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Switched to category %s"),
			*GetCategoryName(Category).ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Invalid category index %d"), CategoryIndex);
	}
}

void UMainMenuSettingWidget::NavigateToNextCategory()
{
	int32 CurrentEnumValue = static_cast<int32>(GetCurrentCategory());
	int32 NextEnumValue = (CurrentEnumValue % 5) + 1; // Cycle through 1-5
	SwitchToCategory(static_cast<ESettingsCategory>(NextEnumValue));
}

void UMainMenuSettingWidget::NavigateToPreviousCategory()
{
	int32 CurrentEnumValue = static_cast<int32>(GetCurrentCategory());
	int32 PrevEnumValue = (CurrentEnumValue == 1) ? 5 : (CurrentEnumValue - 1);
	SwitchToCategory(static_cast<ESettingsCategory>(PrevEnumValue));
}

ESettingsCategory UMainMenuSettingWidget::GetCurrentCategory() const
{
	return static_cast<ESettingsCategory>(CurrentCategoryIndex + 1);
}

void UMainMenuSettingWidget::SetWidgetManager(AWidgetManager* InWidgetManager)
{
	WidgetManager = InWidgetManager;
    
	if (WidgetManager)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: WidgetManager set successfully"));

		InitializeTabWidgets();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: WidgetManager is NULL!"));
	}
}

void UMainMenuSettingWidget::InitializeTabWidgets()
{ 
	if (!WidgetManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: WidgetManager not set, cannot initialize tabs"));
		return;
	}
	
	if (GraphicWidget)
	{
		GraphicWidget->SetWidgetManager(WidgetManager);
		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: GraphicWidget initialized with WidgetManager"));
	}
}

void UMainMenuSettingWidget::UpdateCategoryButtons(ESettingsCategory ActiveCategory)
{
	// Set all buttons to inactive first
	SetButtonActive(GameplayButton, false);
	SetButtonActive(GraphicsButton, false);
	SetButtonActive(AudioButton, false);
	SetButtonActive(ControlsButton, false);
	SetButtonActive(AccessibilityButton, false);

	// Set active button
	switch (ActiveCategory)
	{
	case ESettingsCategory::Gameplay:
		SetButtonActive(GameplayButton, true);
		break;
	case ESettingsCategory::Graphics:
		SetButtonActive(GraphicsButton, true);
		break;
	case ESettingsCategory::Audio:
		SetButtonActive(AudioButton, true);
		break;
	case ESettingsCategory::Controls:
		SetButtonActive(ControlsButton, true);
		break;
	case ESettingsCategory::Accessibility:
		SetButtonActive(AccessibilityButton, true);
		break;
	}
}

void UMainMenuSettingWidget::UpdateCategoryTitle(ESettingsCategory Category)
{
	if (CategoryTitleText)
	{
		CategoryTitleText->SetText(GetCategoryName(Category));
	}
}

void UMainMenuSettingWidget::UpdateCategoryIcon(ESettingsCategory Category)
{
	// This can be implemented in Blueprint by binding to OnCategoryChanged
	// and changing the icon texture based on category
}

FText UMainMenuSettingWidget::GetCategoryName(ESettingsCategory Category) const
{
	switch (Category)
	{
	case ESettingsCategory::Gameplay:
		return FText::FromString(TEXT("GAMEPLAY"));
	case ESettingsCategory::Graphics:
		return FText::FromString(TEXT("GRAPHICS"));
	case ESettingsCategory::Audio:
		return FText::FromString(TEXT("AUDIO"));
	case ESettingsCategory::Controls:
		return FText::FromString(TEXT("CONTROLS"));
	case ESettingsCategory::Accessibility:
		return FText::FromString(TEXT("ACCESSIBILITY"));
	default:
		return FText::FromString(TEXT("SETTINGS"));
	}
}

void UMainMenuSettingWidget::SetButtonActive(UButton* Button, bool bActive)
{
	if (!Button)
		return;

	// Visual changes should be done in Blueprint using style
	// This is for any C++ logic needed
	Button->SetIsEnabled(true);

	// You can add custom styling logic here if needed
}

// ===== UNSAVED CHANGES TRACKING =====

void UMainMenuSettingWidget::MarkSettingsChanged()
{
	if (!bHasUnsavedChanges)
	{
		bHasUnsavedChanges = true;
		UpdateUnsavedIndicator();

		// Start auto-save timer
		StartAutoSaveTimer();

		// Play warning animation
		PlayUnsavedWarningAnimation();

		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Settings marked as changed"));
	}
}

void UMainMenuSettingWidget::ClearUnsavedChanges()
{
	bHasUnsavedChanges = false;
	UpdateUnsavedIndicator();
	StopAutoSaveTimer();

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Unsaved changes cleared"));
}

void UMainMenuSettingWidget::CheckForUnsavedChanges()
{
	// Check if current settings differ from saved settings
	if (SettingsSubsystem)
	{
		// Implementation depends on your SettingsSubsystem
		// Compare PendingSettings with saved settings
	}
}

void UMainMenuSettingWidget::ShowUnsavedChangesDialog()
{
	UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Showing unsaved changes dialog"));

	// TODO: Create proper confirmation dialog widget
	// For now, just ask to save
	OnUnsavedChangesDialogResponse(true);
}

void UMainMenuSettingWidget::OnUnsavedChangesDialogResponse(bool bSaveChanges)
{
	if (bSaveChanges)
	{
		OnApplyAllButtonClicked();

		// Wait for settings to apply before closing
		UWorld* World = GetWorld();
		if (World)
		{
			FTimerHandle CloseDelayHandle;
			World->GetTimerManager().SetTimer(CloseDelayHandle, [this]()
				{
					CloseSettingsMenu(true);
				}, 0.5f, false);
		}
	}
	else
	{
		CloseSettingsMenu(true);
	}
}

void UMainMenuSettingWidget::UpdateUnsavedIndicator()
{
	if (UnsavedChangesText)
	{
		UnsavedChangesText->SetVisibility(
			bHasUnsavedChanges ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (UnsavedIndicator)
	{
		UnsavedIndicator->SetVisibility(
			bHasUnsavedChanges ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMainMenuSettingWidget::CloseSettingsMenu(bool bIsLobby)
{
	const auto PlayerCon = GetOwningPlayer();
	if (PlayerCon)
	{
		RestorePlayerInputState();
	}
	
	PlayerCon->bShowMouseCursor = true;
	
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(this->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayUISound("ButtonClick");
	OnBackClicked.Broadcast();
	SetVisibility(ESlateVisibility::Collapsed);
}

// ===== CHANGE TRACKING =====

void UMainMenuSettingWidget::TrackSettingChange(const FString& SettingName, const FString& OldValue, const FString& NewValue)
{
	FSettingChange Change;
	Change.SettingName = SettingName;
	Change.OldValue = OldValue;
	Change.NewValue = NewValue;
	Change.ChangeTime = FDateTime::Now();

	RecentChanges.Add(Change);

	// Limit the number of tracked changes
	if (RecentChanges.Num() > MaxRecentChanges)
	{
		RecentChanges.RemoveAt(0);
	}

	UpdateUndoButton();
	MarkSettingsChanged();

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Tracked change - %s: %s -> %s"),
		*SettingName, *OldValue, *NewValue);
}

void UMainMenuSettingWidget::UndoLastChange()
{
	if (!CanUndo() || !SettingsSubsystem)
		return;

	FSettingChange LastChange = RecentChanges.Last();
	RecentChanges.RemoveAt(RecentChanges.Num() - 1);

	// Revert the setting to old value
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Undoing change - %s: %s"),
		*LastChange.SettingName, *LastChange.OldValue);

	// TODO: Implement actual revert logic based on SettingName
	// This depends on your SettingsSubsystem implementation

	UpdateUndoButton();
	RefreshAllWidgets();

	ShowSuccessNotification(TEXT("Change reverted"));
}

bool UMainMenuSettingWidget::CanUndo() const
{
	return RecentChanges.Num() > 0;
}

void UMainMenuSettingWidget::UpdateUndoButton()
{
	if (UndoButton)
	{
		UndoButton->SetIsEnabled(CanUndo());
	}
}

// ===== GRAPHICS PRESETS =====

void UMainMenuSettingWidget::ApplyGraphicsPreset(EE_GraphicsQuality Preset)
{
	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Cannot apply preset - SettingsSubsystem is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Applying graphics preset %d"), static_cast<int32>(Preset));

	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Cannot get GameUserSettings"));
		return;
	}

	switch (Preset)
	{
	case EE_GraphicsQuality::Low:
		GameSettings->SetOverallScalabilityLevel(0);
		break;
	case EE_GraphicsQuality::Medium:
		GameSettings->SetOverallScalabilityLevel(1);
		break;
	case EE_GraphicsQuality::High:
		GameSettings->SetOverallScalabilityLevel(2);
		break;
	case EE_GraphicsQuality::Ultra:
		GameSettings->SetOverallScalabilityLevel(3);
		break;
	case EE_GraphicsQuality::Custom:
		// Keep current settings
		break;
	}

	MarkSettingsChanged();
	RefreshAllWidgets();

	ShowSuccessNotification(FString::Printf(TEXT("Graphics preset applied: %s"),
		*UEnum::GetValueAsString(Preset)));
}

void UMainMenuSettingWidget::AutoDetectOptimalSettings()
{
	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Cannot auto-detect - SettingsSubsystem is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Auto-detecting optimal settings"));

	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (GameSettings)
	{
		// Show loading during benchmark
		ShowLoadingScreen();

		GameSettings->RunHardwareBenchmark();
		GameSettings->ApplyHardwareBenchmarkResults();

		HideLoadingScreen();

		MarkSettingsChanged();
		RefreshAllWidgets();

		PlayUISound("Apply");
		ShowSuccessNotification(TEXT("Optimal settings detected and applied"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Cannot get GameUserSettings for auto-detect"));
		ShowErrorNotification(TEXT("Failed to auto-detect settings"));
	}
}

// ===== SEARCH & FILTER =====

void UMainMenuSettingWidget::FilterSettingsBySearch(const FString& SearchText)
{
	if (SearchText.IsEmpty())
	{
		ClearSearch();
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Filtering settings by '%s'"), *SearchText);

	// Filter settings in each category widget
	// TODO: Implement search functionality in child widgets

	if (GameplayWidget)
	{
		// GameplayWidget->FilterBySearch(SearchText);
	}

	if (GraphicWidget)
	{
		// GraphicWidget->FilterBySearch(SearchText);
	}

	if (AudioWidget)
	{
		// AudioWidget->FilterBySearch(SearchText);
	}

	if (ControlWidget)
	{
		// ControlWidget->FilterBySearch(SearchText);
	}

	if (AccessibilityWidget)
	{
		// AccessibilityWidget->FilterBySearch(SearchText);
	}
}

void UMainMenuSettingWidget::ClearSearch()
{
	CurrentSearchText.Empty();

	if (SearchBox)
	{
		SearchBox->SetText(FText::GetEmpty());
	}

	// Clear filters on all widgets
	FilterSettingsBySearch("");
}

// ===== VALIDATION =====

bool UMainMenuSettingWidget::ValidateAllSettings()
{
	ValidationErrors.Empty();

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Validating all settings..."));

	// Validate Graphics Settings
	if (GraphicWidget)
	{
		TArray<FString> GraphicsErrors = GraphicWidget->ValidateSettings();
		if (GraphicsErrors.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("  - Graphics: %d errors"), GraphicsErrors.Num());
			ValidationErrors.Append(GraphicsErrors);
		}
	}

	// Validate Audio Settings
	if (AudioWidget)
	{
		TArray<FString> AudioErrors = AudioWidget->ValidateSettings();
		if (AudioErrors.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("  - Audio: %d errors"), AudioErrors.Num());
			ValidationErrors.Append(AudioErrors);
		}
	}

	// Validate Gameplay Settings
	if (GameplayWidget)
	{
		TArray<FString> GameplayErrors = GameplayWidget->ValidateSettings();
		if (GameplayErrors.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("  - Gameplay: %d errors"), GameplayErrors.Num());
			ValidationErrors.Append(GameplayErrors);
		}
	}

	// Validate Control Settings
	if (ControlWidget)
	{
		TArray<FString> ControlErrors = ControlWidget->ValidateSettings();
		if (ControlErrors.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("  - Control: %d errors"), ControlErrors.Num());
			ValidationErrors.Append(ControlErrors);
		}
	}

	// Validate Accessibility Settings
	if (AccessibilityWidget)
	{
		TArray<FString> AccessibilityErrors = AccessibilityWidget->ValidateSettings();
		if (AccessibilityErrors.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("  - Accessibility: %d errors"), AccessibilityErrors.Num());
			ValidationErrors.Append(AccessibilityErrors);
		}
	}

	bool bIsValid = ValidationErrors.Num() == 0;

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Validation failed with %d total errors"),
			ValidationErrors.Num());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: All settings validated successfully"));
	}

	return bIsValid;
}

void UMainMenuSettingWidget::ValidateSettingsForHardware()
{
	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameSettings)
		return;

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Validating settings for hardware"));

	// TODO: Check if settings might cause performance issues
	// Compare current settings with hardware benchmark results
}

void UMainMenuSettingWidget::ShowValidationErrors(const TArray<FString>& Errors)
{
	PlayUISound("Error");

	FString ErrorMessage = TEXT("Validation Errors:\n");
	for (int32 i = 0; i < Errors.Num(); ++i)
	{
		ErrorMessage += FString::Printf(TEXT("%d. %s\n"), i + 1, *Errors[i]);
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Validation error %d - %s"), i + 1, *Errors[i]);
	}

	ShowErrorNotification(ErrorMessage);
}

void UMainMenuSettingWidget::ShowSuccessNotification(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("Success: %s"), *Message);

	if (NotificationWidget)
	{
		NotificationWidget->ShowSuccess(FText::FromString(Message));
	}
}

void UMainMenuSettingWidget::ShowErrorNotification(const FString& Message)
{
	UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Message);

	if (NotificationWidget)
	{
		NotificationWidget->ShowError(FText::FromString(Message));
	}
}

// ===== PERFORMANCE ESTIMATION =====

void UMainMenuSettingWidget::UpdatePerformanceEstimate()
{
	if (!PerformanceEstimateText)
		return;

	FText EstimateText = GetPerformanceImpactText();
	PerformanceEstimateText->SetText(EstimateText);
}

FText UMainMenuSettingWidget::GetPerformanceImpactText() const
{
	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameSettings)
		return FText::FromString(TEXT("Unknown"));

	int32 OverallQuality = GameSettings->GetOverallScalabilityLevel();

	switch (OverallQuality)
	{
	case 0:
		return FText::FromString(TEXT("Performance: Very High FPS (Low Quality)"));
	case 1:
		return FText::FromString(TEXT("Performance: High FPS (Medium Quality)"));
	case 2:
		return FText::FromString(TEXT("Performance: Medium FPS (High Quality)"));
	case 3:
		return FText::FromString(TEXT("Performance: Lower FPS (Ultra Quality)"));
	default:
		return FText::FromString(TEXT("Performance: Custom Settings"));
	}
}

// ===== ANIMATIONS =====

void UMainMenuSettingWidget::PlayCategorySwitchAnimation()
{
	if (CategorySwitchAnimation)
	{
		PlayAnimation(CategorySwitchAnimation);
	}
}

void UMainMenuSettingWidget::PlayUnsavedWarningAnimation()
{
	if (UnsavedWarningAnimation)
	{
		PlayAnimation(UnsavedWarningAnimation);
	}
}

void UMainMenuSettingWidget::PlayApplySuccessAnimation()
{
	if (ApplySuccessAnimation)
	{
		PlayAnimation(ApplySuccessAnimation);
	}
}

// ===== ASYNC OPERATIONS =====

void UMainMenuSettingWidget::CollectSettingsFromWidgets()
{
	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Cannot collect settings - SettingsSubsystem is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Collecting settings from all widgets..."));

	// Get current settings from subsystem as base
	PendingSettings = SettingsSubsystem->GetAllSettings();

	// Collect from each widget
	if (GameplayWidget)
	{
		PendingSettings.GameplaySettings = GameplayWidget->GetCurrentSettings();
		UE_LOG(LogTemp, Log, TEXT("  - Collected Gameplay settings"));
	}

	if (GraphicWidget)
	{
		PendingSettings.GraphicsSettings = GraphicWidget->GetCurrentSettings();
		UE_LOG(LogTemp, Log, TEXT("  - Collected Graphics settings"));
	}

	if (AudioWidget)
	{
		PendingSettings.AudioSettings = AudioWidget->GetCurrentSettings();
		UE_LOG(LogTemp, Log, TEXT("  - Collected Audio settings"));
	}

	if (ControlWidget)
	{
		PendingSettings.ControlSettings = ControlWidget->GetCurrentSettings();
		UE_LOG(LogTemp, Log, TEXT("  - Collected Control settings"));
	}

	if (AccessibilityWidget)
	{
		PendingSettings.AccessibilitySettings = AccessibilityWidget->GetCurrentSettings();
		UE_LOG(LogTemp, Log, TEXT("  - Collected Accessibility settings"));
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Finished collecting settings"));
}

void UMainMenuSettingWidget::AsyncApplySettings()
{
	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Cannot apply - SettingsSubsystem is null"));
		HideLoadingScreen();
		bIsApplyingSettings = false;
		ShowErrorNotification(TEXT("Settings system not available"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Applying settings asynchronously..."));

	// Apply settings through subsystem
	bool bSuccess = SettingsSubsystem->ApplyAllSettings(PendingSettings);

	// Hide loading screen
	HideLoadingScreen();

	// Handle result
	OnSettingsApplied_Internal(bSuccess);
}

void UMainMenuSettingWidget::OnSettingsApplied_Internal(bool bSuccess)
{
	bIsApplyingSettings = false;

	if (bSuccess)
	{
		// Save to file
		if (SettingsSubsystem)
		{
			SettingsSubsystem->SaveAllSettings();
		}

		// Clear unsaved changes flag
		ClearUnsavedChanges();

		// Clear change history since we've saved
		RecentChanges.Empty();
		UpdateUndoButton();

		// Play success animation and sound
		PlayApplySuccessAnimation();
		PlayUISound("Apply");

		// Broadcast event
		OnSettingsApplied.Broadcast();

		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Settings applied and saved successfully"));

		// Show success notification
		ShowSuccessNotification(TEXT("Settings saved successfully!"));
	}
	else
	{
		PlayUISound("Error");
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Failed to apply settings"));

		// Show error notification
		ShowErrorNotification(TEXT("Failed to apply settings. Please try again."));
	}
}

// ===== AUTO-SAVE =====

void UMainMenuSettingWidget::StartAutoSaveTimer()
{
	StopAutoSaveTimer();

	UWorld* World = GetWorld();
	if (World && AutoSaveDelay > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			AutoSaveTimerHandle,
			this,
			&UMainMenuSettingWidget::OnAutoSaveTimerElapsed,
			AutoSaveDelay,
			false
		);

		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Auto-save timer started (%.1fs)"), AutoSaveDelay);
	}
}

void UMainMenuSettingWidget::StopAutoSaveTimer()
{
	UWorld* World = GetWorld();
	if (World && AutoSaveTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
		AutoSaveTimerHandle.Invalidate();
		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Auto-save timer stopped"));
	}
}

void UMainMenuSettingWidget::OnAutoSaveTimerElapsed()
{
	if (bHasUnsavedChanges && !bIsApplyingSettings)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Auto-saving settings"));
		OnApplyAllButtonClicked();
	}
}

// ===== TOOLTIPS & HELP =====

void UMainMenuSettingWidget::ShowSettingsHelp(ESettingsCategory Category)
{
	FString HelpText;

	switch (Category)
	{
	case ESettingsCategory::Gameplay:
		HelpText = TEXT("Gameplay settings control game mechanics, difficulty, and player preferences.");
		break;
	case ESettingsCategory::Graphics:
		HelpText = TEXT("Graphics settings affect visual quality and performance. Higher settings provide better visuals but may reduce FPS.");
		break;
	case ESettingsCategory::Audio:
		HelpText = TEXT("Audio settings control volume levels for different sound categories and audio quality.");
		break;
	case ESettingsCategory::Controls:
		HelpText = TEXT("Controls settings allow you to customize key bindings and input sensitivity.");
		break;
	case ESettingsCategory::Accessibility:
		HelpText = TEXT("Accessibility settings help make the game more comfortable and playable for all users.");
		break;
	default:
		HelpText = TEXT("Settings help you customize your game experience.");
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Help - %s"), *HelpText);

	// TODO: Show help dialog with this text
	ShowSuccessNotification(HelpText);
}

FText UMainMenuSettingWidget::GetSettingTooltip(const FString& SettingName) const
{
	// TODO: Load tooltip from data table or localization
	return FText::FromString(FString::Printf(TEXT("Tooltip for %s"), *SettingName));
}

// ===== PROFILES =====

void UMainMenuSettingWidget::SaveCustomProfile(const FString& ProfileName)
{
	if (!SettingsSubsystem || ProfileName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Cannot save profile - invalid parameters"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Saving custom profile '%s'"), *ProfileName);

	// TODO: Implement profile saving
	// SettingsSubsystem->SaveProfile(ProfileName, PendingSettings);

	ShowSuccessNotification(FString::Printf(TEXT("Profile '%s' saved"), *ProfileName));
}

void UMainMenuSettingWidget::LoadCustomProfile(const FString& ProfileName)
{
	if (!SettingsSubsystem || ProfileName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Cannot load profile - invalid parameters"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Loading custom profile '%s'"), *ProfileName);

	// TODO: Implement profile loading
	// FS_AllSettings ProfileSettings = SettingsSubsystem->LoadProfile(ProfileName);
	// if (ProfileSettings is valid)
	// {
	//     PendingSettings = ProfileSettings;
	//     RefreshAllWidgets();
	// }

	RefreshAllWidgets();
	ShowSuccessNotification(FString::Printf(TEXT("Profile '%s' loaded"), *ProfileName));
}

// ===== SUBSYSTEM DELEGATES =====

void UMainMenuSettingWidget::BindSubsystemDelegates()
{
	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Cannot bind delegates - SettingsSubsystem is null"));
		return;
	}

	SettingsSubsystem->OnGraphicsSettingsChanged.AddDynamic(
		this, &UMainMenuSettingWidget::OnGraphicsSettingsChanged_Internal);
	SettingsSubsystem->OnAudioSettingsChanged.AddDynamic(
		this, &UMainMenuSettingWidget::OnAudioSettingsChanged_Internal);
	SettingsSubsystem->OnGameplaySettingsChanged.AddDynamic(
		this, &UMainMenuSettingWidget::OnGameplaySettingsChanged_Internal);
	SettingsSubsystem->OnControlSettingsChanged.AddDynamic(
		this, &UMainMenuSettingWidget::OnControlSettingsChanged_Internal);
	SettingsSubsystem->OnAccessibilitySettingsChanged.AddDynamic(
		this, &UMainMenuSettingWidget::OnAccessibilitySettingsChanged_Internal);

	SettingsSubsystem->OnSettingsApplyFailed.AddDynamic(
		this, &UMainMenuSettingWidget::OnSettingsApplyFailed_Internal);
	SettingsSubsystem->OnSettingsConfirmationRequired.AddDynamic(
		this, &UMainMenuSettingWidget::OnSettingsConfirmationRequired_Internal);
	SettingsSubsystem->OnSettingsConfirmed.AddDynamic(
		this, &UMainMenuSettingWidget::OnSettingsConfirmed_Internal);
	SettingsSubsystem->OnSettingsReverted.AddDynamic(
		this, &UMainMenuSettingWidget::OnSettingsReverted_Internal);

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Bound subsystem delegates"));
}

void UMainMenuSettingWidget::UnbindSubsystemDelegates()
{
	if (!SettingsSubsystem)
		return;

	SettingsSubsystem->OnGraphicsSettingsChanged.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnGraphicsSettingsChanged_Internal);
	SettingsSubsystem->OnAudioSettingsChanged.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnAudioSettingsChanged_Internal);
	SettingsSubsystem->OnGameplaySettingsChanged.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnGameplaySettingsChanged_Internal);
	SettingsSubsystem->OnControlSettingsChanged.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnControlSettingsChanged_Internal);
	SettingsSubsystem->OnAccessibilitySettingsChanged.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnAccessibilitySettingsChanged_Internal);

	SettingsSubsystem->OnSettingsApplyFailed.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnSettingsApplyFailed_Internal);
	SettingsSubsystem->OnSettingsConfirmationRequired.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnSettingsConfirmationRequired_Internal);
	SettingsSubsystem->OnSettingsConfirmed.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnSettingsConfirmed_Internal);
	SettingsSubsystem->OnSettingsReverted.RemoveDynamic(
		this, &UMainMenuSettingWidget::OnSettingsReverted_Internal);

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Unbound subsystem delegates"));
}

void UMainMenuSettingWidget::BindChildWidgetEvents()
{
	// TODO: Implement if child widgets have OnSettingChanged events
	// Example:
	// if (GameplayWidget)
	// {
	//     GameplayWidget->OnSettingChanged.AddDynamic(this, &UMainMenuSettingWidget::OnChildWidgetSettingChanged);
	// }

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Child widget events binding (not implemented)"));
}

void UMainMenuSettingWidget::UnbindChildWidgetEvents()
{
	// TODO: Implement if child widgets have OnSettingChanged events
	// Example:
	// if (GameplayWidget)
	// {
	//     GameplayWidget->OnSettingChanged.RemoveDynamic(this, &UMainMenuSettingWidget::OnChildWidgetSettingChanged);
	// }
}

void UMainMenuSettingWidget::OnChildWidgetSettingChanged()
{
	MarkSettingsChanged();
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Child widget setting changed"));
}

// ===== SUBSYSTEM EVENT HANDLERS =====

void UMainMenuSettingWidget::OnGraphicsSettingsChanged_Internal(const FS_GraphicsSettings& NewSettings)
{
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Graphics settings changed externally"));

	if (GraphicWidget)
	{
		GraphicWidget->LoadSettings(NewSettings);
	}
	UpdatePerformanceEstimate();
}

void UMainMenuSettingWidget::OnAudioSettingsChanged_Internal(const FS_AudioSettings& NewSettings)
{
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Audio settings changed externally"));

	if (AudioWidget)
	{
		AudioWidget->LoadSettings(NewSettings);
	}
}

void UMainMenuSettingWidget::OnGameplaySettingsChanged_Internal(const FS_GameplaySettings& NewSettings)
{
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Gameplay settings changed externally"));

	if (GameplayWidget)
	{
		GameplayWidget->LoadSettings(NewSettings);
	}
}

void UMainMenuSettingWidget::OnControlSettingsChanged_Internal(const FS_ControlSettings& NewSettings)
{
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Control settings changed externally"));

	if (ControlWidget)
	{
		ControlWidget->LoadSettings(NewSettings);
	}
}

void UMainMenuSettingWidget::OnAccessibilitySettingsChanged_Internal(const FS_AccessibilitySettings& NewSettings)
{
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Accessibility settings changed externally"));

	if (AccessibilityWidget)
	{
		AccessibilityWidget->LoadSettings(NewSettings);
	}
}

void UMainMenuSettingWidget::OnSettingsApplyFailed_Internal(const FString& FailureReason)
{
	bIsApplyingSettings = false;
	HideLoadingScreen();

	UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Settings apply failed - %s"), *FailureReason);

	ShowErrorNotification(FString::Printf(TEXT("Failed to apply settings: %s"), *FailureReason));
	PlayUISound("Error");
}

void UMainMenuSettingWidget::OnSettingsConfirmationRequired_Internal(float ConfirmationTime)
{
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Settings confirmation required (%.1fs)"), ConfirmationTime);

	ShowConfirmationDialog(ConfirmationTime);
}

void UMainMenuSettingWidget::OnSettingsConfirmed_Internal()
{
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Settings confirmed"));

	ClearUnsavedChanges();
	ShowSuccessNotification(TEXT("Settings confirmed and saved!"));
	PlayApplySuccessAnimation();
}

void UMainMenuSettingWidget::OnSettingsReverted_Internal()
{
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Settings reverted"));

	RefreshAllWidgets();
	ShowErrorNotification(TEXT("Settings reverted to previous values"));
	PlayUISound("Error");
}

// ===== CONFIRMATION DIALOG SYSTEM =====

void UMainMenuSettingWidget::ShowConfirmationDialog(float CountdownTime)
{
	if (!ConfirmationDialogClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: ConfirmationDialogClass not set"));
		return;
	}

	RemainingConfirmationTime = CountdownTime;

	// Remove existing dialog if any
	if (ActiveConfirmationDialog)
	{
		ActiveConfirmationDialog->RemoveFromParent();
		ActiveConfirmationDialog = nullptr;
	}

	// Create and show new dialog
	ActiveConfirmationDialog = CreateWidget<UConfirmationDialogWidget>(GetOwningPlayer(), ConfirmationDialogClass);
	if (ActiveConfirmationDialog)
	{
		ActiveConfirmationDialog->AddToViewport(999); // High Z-order

		// TODO: Set dialog message and bind callbacks
		// ActiveConfirmationDialog->SetMessage(TEXT("Keep these settings?"));
		// ActiveConfirmationDialog->OnConfirmed.AddDynamic(this, &UMainMenuSettingWidget::OnConfirmationAccepted);
		// ActiveConfirmationDialog->OnDeclined.AddDynamic(this, &UMainMenuSettingWidget::OnConfirmationDeclined);

		// Start countdown timer
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				ConfirmationCountdownHandle,
				this,
				&UMainMenuSettingWidget::UpdateConfirmationCountdown,
				1.0f,
				true
			);
		}

		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Showing confirmation dialog (%.1fs)"), CountdownTime);
	}
}

void UMainMenuSettingWidget::UpdateConfirmationCountdown()
{
	RemainingConfirmationTime -= 1.0f;

	// Update dialog with remaining time
	if (ActiveConfirmationDialog)
	{
		// TODO: Update countdown display
		// ActiveConfirmationDialog->UpdateCountdown(RemainingConfirmationTime);
	}

	// Auto-decline on timeout
	if (RemainingConfirmationTime <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Confirmation timeout, reverting settings"));
		OnConfirmationDeclined();
	}
}

void UMainMenuSettingWidget::OnConfirmationAccepted()
{
	// Stop countdown timer
	UWorld* World = GetWorld();
	if (World && ConfirmationCountdownHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(ConfirmationCountdownHandle);
		ConfirmationCountdownHandle.Invalidate();
	}

	// Confirm settings in subsystem
	if (SettingsSubsystem)
	{
		// TODO: Implement ConfirmPendingSettings in subsystem
		// SettingsSubsystem->ConfirmPendingSettings();
	}

	// Remove dialog
	if (ActiveConfirmationDialog)
	{
		ActiveConfirmationDialog->RemoveFromParent();
		ActiveConfirmationDialog = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Settings confirmed by user"));
}

void UMainMenuSettingWidget::OnConfirmationDeclined()
{
	// Stop countdown timer
	UWorld* World = GetWorld();
	if (World && ConfirmationCountdownHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(ConfirmationCountdownHandle);
		ConfirmationCountdownHandle.Invalidate();
	}

	// Revert settings in subsystem
	if (SettingsSubsystem)
	{
		// TODO: Implement RevertPendingSettings in subsystem
		// SettingsSubsystem->RevertPendingSettings();
	}

	// Remove dialog
	if (ActiveConfirmationDialog)
	{
		ActiveConfirmationDialog->RemoveFromParent();
		ActiveConfirmationDialog = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Settings reverted (declined/timeout)"));
}

// ===== LOADING SCREEN =====

void UMainMenuSettingWidget::ShowLoadingScreen()
{
	if (!LoadingScreenClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: LoadingScreenClass not set"));
		return;
	}

	// Remove existing loading screen if any
	if (ActiveLoadingScreen)
	{
		ActiveLoadingScreen->RemoveFromParent();
		ActiveLoadingScreen = nullptr;
	}

	// Create and show loading screen
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		ActiveLoadingScreen = CreateWidget<ULoadingScreenWidget>(PC, LoadingScreenClass);
		if (ActiveLoadingScreen)
		{
			ActiveLoadingScreen->AddToViewport(1000); // Very high Z-order
			UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Showing loading screen"));
		}
	}
}

void UMainMenuSettingWidget::HideLoadingScreen()
{
	if (ActiveLoadingScreen)
	{
		ActiveLoadingScreen->RemoveFromParent();
		ActiveLoadingScreen = nullptr;
		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Hiding loading screen"));
	}
}

// ===== SOUND EFFECTS =====

void UMainMenuSettingWidget::PlayUISound(const FString& SoundName)
{
	USoundBase* SoundToPlay = nullptr;

	if (SoundName == "ButtonClick" && ButtonClickSound)
	{
		SoundToPlay = ButtonClickSound;
	}
	else if (SoundName == "CategorySwitch" && CategorySwitchSound)
	{
		SoundToPlay = CategorySwitchSound;
	}
	else if (SoundName == "Apply" && ApplySound)
	{
		SoundToPlay = ApplySound;
	}
	else if (SoundName == "Error" && ErrorSound)
	{
		SoundToPlay = ErrorSound;
	}

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(this, SoundToPlay);
	}
}

// ===== ACCESSIBILITY =====

void UMainMenuSettingWidget::AnnounceCurrentCategory()
{
	ESettingsCategory CurrentCategory = GetCurrentCategory();
	FString AnnouncementText = FString::Printf(
		TEXT("Switched to %s settings"),
		*GetCategoryName(CurrentCategory).ToString()
	);

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: %s"), *AnnouncementText);

	// TODO: Implement screen reader announcement
	// If your engine has accessibility features, announce this text
}

void UMainMenuSettingWidget::AnnounceSettingChange(const FString& SettingName, const FString& NewValue)
{
	FString AnnouncementText = FString::Printf(
		TEXT("%s changed to %s"),
		*SettingName,
		*NewValue
	);

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: %s"), *AnnouncementText);

	// TODO: Implement screen reader announcement
}

// ===== PUBLIC UTILITY FUNCTIONS =====

void UMainMenuSettingWidget::RefreshAllWidgets()
{
	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Cannot refresh - SettingsSubsystem is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Refreshing all widgets..."));

	FS_AllSettings CurrentSettings = SettingsSubsystem->GetAllSettings();

	if (GameplayWidget)
	{
		GameplayWidget->LoadSettings(CurrentSettings.GameplaySettings);
	}

	if (GraphicWidget)
	{
		GraphicWidget->LoadSettings(CurrentSettings.GraphicsSettings);
	}

	if (AudioWidget)
	{
		AudioWidget->LoadSettings(CurrentSettings.AudioSettings);
	}

	if (ControlWidget)
	{
		ControlWidget->LoadSettings(CurrentSettings.ControlSettings);
	}

	if (AccessibilityWidget)
	{
		AccessibilityWidget->LoadSettings(CurrentSettings.AccessibilitySettings);
	}

	UpdatePerformanceEstimate();

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: All widgets refreshed"));
}

void UMainMenuSettingWidget::ForceApplySettings()
{
	if (SettingsSubsystem)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Force applying settings"));
		OnApplyAllButtonClicked();
	}
}

void UMainMenuSettingWidget::ShowSettingsMenu(bool bIsLobby)
{
	bOpenedFromLobby = bIsLobby;
	
	const auto PC = GetOwningPlayer();
	if (PC)
	{
		bSavedInputStateValid = PC->bShowMouseCursor;
		bSavedInputStateValid = true;

		if (PC->bShowMouseCursor)
		{
			SavedInputMode = ESavedInputMode::GameAndUI;
		}
		else
		{
			SavedInputMode = ESavedInputMode::GameOnly;
		}
		
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	
	SetVisibility(ESlateVisibility::Visible);
	PlayUISound("ButtonClick");
}

void UMainMenuSettingWidget::RestorePlayerInputState()
{
	const auto PC = GetOwningPlayer();
	if (!PC) return;

	if (bSavedInputStateValid)
	{
		switch (SavedInputMode)
		{
		case ESavedInputMode::GameOnly:
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
			break;
		case ESavedInputMode::UIOnly:
			{
				FInputModeUIOnly InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;
			}
			break;
		case ESavedInputMode::GameAndUI:
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = bSavedShowMouseCursor;
			}
			break;
		}
		bSavedInputStateValid = false;
	}
	else
	{
		if (bOpenedFromLobby)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
		else
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
}