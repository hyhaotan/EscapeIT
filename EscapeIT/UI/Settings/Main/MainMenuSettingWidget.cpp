// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuSettingWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "CommonTextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "EscapeIT/UI/Settings/Tab/GameplayWidget.h"
#include "EscapeIT/UI/Settings/Tab/GraphicWidget.h"
#include "EscapeIT/UI/Settings/Tab/AudioWidget.h"
#include "EscapeIT/UI/Settings/Tab/ControlWidget.h"
#include "EscapeIT/UI/Settings/Tab/AccessibilityWidget.h"
#include "EscapeIT/Subsystem/SettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "GameFramework/GameUserSettings.h"
#include "EscapeIT/UI/NotificationWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

UMainMenuSettingWidget::UMainMenuSettingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentCategoryIndex(0)
	, bHasUnsavedChanges(false)
	, bIsApplyingSettings(false)
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
	}

	// Bind all button events
	BindButtonEvents();

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
}

void UMainMenuSettingWidget::NativeDestruct()
{
	// Stop auto-save timer
	StopAutoSaveTimer();

	// Unbind all button events
	UnbindButtonEvents();

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

	TObjectPtr<APlayerController> PlayerCon = GetOwningPlayer();
	if (PlayerCon)
	{
		PlayerCon->SetInputMode(FInputModeGameOnly());
		PlayerCon->bShowMouseCursor = false;
	}

	PlayUISound("ButtonClick");
	OnBackClicked.Broadcast();
	RemoveFromParent();
}

void UMainMenuSettingWidget::OnApplyAllButtonClicked()
{
	if (!SettingsSubsystem || bIsApplyingSettings)
		return;

	PlayUISound("Apply");

	CollectSettingsFromWidgets();

	AsyncApplySettings();
}

void UMainMenuSettingWidget::OnResetAllButtonClicked()
{
	if (!SettingsSubsystem)
		return;

	PlayUISound("ButtonClick");

	// Reset all settings to default
	SettingsSubsystem->ResetSettingsToDefault();

	// Refresh all widgets
	RefreshAllWidgets();

	// Clear unsaved changes
	ClearUnsavedChanges();
	RecentChanges.Empty();
	UpdateUndoButton();

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

	// Switch to the appropriate widget
	int32 CategoryIndex = static_cast<int32>(Category);
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

void UMainMenuSettingWidget::NavigateToNextCategory()
{
	int32 NextIndex = (CurrentCategoryIndex + 1) % 5;
	SwitchToCategory(static_cast<ESettingsCategory>(NextIndex));
}

void UMainMenuSettingWidget::NavigateToPreviousCategory()
{
	int32 PrevIndex = (CurrentCategoryIndex - 1 + 5) % 5;
	SwitchToCategory(static_cast<ESettingsCategory>(PrevIndex));
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
}

// ===== UNSAVED CHANGES TRACKING =====

void UMainMenuSettingWidget::MarkSettingsChanged()
{
	bHasUnsavedChanges = true;
	UpdateUnsavedIndicator();

	// Start auto-save timer
	StartAutoSaveTimer();

	// Play warning animation
	PlayUnsavedWarningAnimation();
}

void UMainMenuSettingWidget::ClearUnsavedChanges()
{
	bHasUnsavedChanges = false;
	UpdateUnsavedIndicator();
	StopAutoSaveTimer();
}

void UMainMenuSettingWidget::CheckForUnsavedChanges()
{
	// This can be called by child widgets when they detect changes
	if (SettingsSubsystem)
	{
		// Check if current settings differ from saved settings
		// Implementation depends on your SettingsSubsystem
	}
}

void UMainMenuSettingWidget::ShowUnsavedChangesDialog()
{
	// Create and show confirmation dialog
	// This should be implemented using your confirmation dialog widget
	UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Unsaved changes detected"));

	// For now, just ask to save
	OnUnsavedChangesDialogResponse(true);
}

void UMainMenuSettingWidget::OnUnsavedChangesDialogResponse(bool bSaveChanges)
{
	if (bSaveChanges)
	{
		OnApplyAllButtonClicked();
	}

	// Then proceed with back action
	OnBackClicked.Broadcast();
	RemoveFromParent();
}

void UMainMenuSettingWidget::UpdateUnsavedIndicator()
{
	if (UnsavedChangesText)
	{
		UnsavedChangesText->SetVisibility(bHasUnsavedChanges ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (UnsavedIndicator)
	{
		UnsavedIndicator->SetVisibility(bHasUnsavedChanges ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

// ===== CHANGE TRACKING =====

// ===== CHANGE TRACKING (tiếp theo) =====

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
	// Implementation depends on your SettingsSubsystem
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Undoing change - %s: %s"),
		*LastChange.SettingName, *LastChange.OldValue);

	UpdateUndoButton();
	RefreshAllWidgets();
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
		return;

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Applying graphics preset"));

	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameSettings)
		return;

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
}

void UMainMenuSettingWidget::AutoDetectOptimalSettings()
{
	if (!SettingsSubsystem)
		return;

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Auto-detecting optimal settings"));

	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (GameSettings)
	{
		GameSettings->RunHardwareBenchmark();
		GameSettings->ApplyHardwareBenchmarkResults();

		MarkSettingsChanged();
		RefreshAllWidgets();

		PlayUISound("Apply");
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

	// Filter settings in each category widget
	if (GameplayWidget)
	{
		// Call filter function on gameplay widget if implemented
	}

	if (GraphicWidget)
	{
		// Call filter function on graphic widget if implemented
	}

	if (AudioWidget)
	{
		// Call filter function on audio widget if implemented
	}

	if (ControlWidget)
	{
		// Call filter function on control widget if implemented
	}

	if (AccessibilityWidget)
	{
		// Call filter function on accessibility widget if implemented
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Filtering settings by '%s'"), *SearchText);
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

	// Validate Graphics Settings
	if (GraphicWidget)
	{
		TArray<FString> GraphicsErrors = GraphicWidget->ValidateSettings();
		ValidationErrors.Append(GraphicsErrors);
	}

	// Validate Audio Settings
	if (AudioWidget)
	{
		TArray<FString> AudioErrors = AudioWidget->ValidateSettings();
		ValidationErrors.Append(AudioErrors);
	}

	// Validate Gameplay Settings
	if (GameplayWidget)
	{
		TArray<FString> GameplayErrors = GameplayWidget->ValidateSettings();
		ValidationErrors.Append(GameplayErrors);
	}

	// Validate Control Settings
	if (ControlWidget)
	{
		TArray<FString> ControlErrors = ControlWidget->ValidateSettings();
		ValidationErrors.Append(ControlErrors);
	}

	// Validate Accessibility Settings
	if (AccessibilityWidget)
	{
		TArray<FString> AccessibilityErrors = AccessibilityWidget->ValidateSettings();
		ValidationErrors.Append(AccessibilityErrors);
	}

	bool bIsValid = ValidationErrors.Num() == 0;

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Validation failed with %d errors"),
			ValidationErrors.Num());
	}

	return bIsValid;
}

void UMainMenuSettingWidget::ValidateSettingsForHardware()
{
	// Check if settings are appropriate for current hardware
	UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameSettings)
		return;

	// This would check if settings might cause performance issues
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Validating settings for hardware"));
}

void UMainMenuSettingWidget::ShowValidationErrors(const TArray<FString>& Errors)
{
	PlayUISound("Error");

	for (const FString& Error : Errors)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuSettingWidget: Validation error - %s"), *Error);
	}

	// Show error dialog or notification
	// Implementation depends on your UI system
}

void UMainMenuSettingWidget::ShowSuccessNotification(const FString& Message)
{
	// Implement your notification system here
	// Example: Show a green checkmark with message
	UE_LOG(LogTemp, Log, TEXT("Success: %s"), *Message);

	// Nếu có notification widget
	if (NotificationWidget)
	{
		NotificationWidget->ShowSuccess(FText::FromString(Message));
	}
}

void UMainMenuSettingWidget::ShowErrorNotification(const FString& Message)
{
	// Implement your notification system here
	// Example: Show a red X with message
	UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Message);

	// Nếu có notification widget
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
		return FText::FromString(TEXT("Performance: Very High FPS"));
	case 1:
		return FText::FromString(TEXT("Performance: High FPS"));
	case 2:
		return FText::FromString(TEXT("Performance: Medium FPS"));
	case 3:
		return FText::FromString(TEXT("Performance: Lower FPS, Best Quality"));
	default:
		return FText::FromString(TEXT("Performance: Custom"));
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
		return;

	// Lấy current settings từ subsystem
	FS_AllSettings CurrentSettings = SettingsSubsystem->GetAllSettings();

	// Collect từ Gameplay Widget
	if (GameplayWidget)
	{
		CurrentSettings.GameplaySettings = GameplayWidget->GetCurrentSettings();
	}

	// Collect từ Graphics Widget
	if (GraphicWidget)
	{
		CurrentSettings.GraphicsSettings = GraphicWidget->GetCurrentSettings();
	}

	// Collect từ Audio Widget
	if (AudioWidget)
	{
		CurrentSettings.AudioSettings = AudioWidget->GetCurrentSettings();
	}

	// Collect từ Control Widget
	if (ControlWidget)
	{
		CurrentSettings.ControlSettings = ControlWidget->GetCurrentSettings();
	}

	// Collect từ Accessibility Widget
	if (AccessibilityWidget)
	{
		CurrentSettings.AccessibilitySettings = AccessibilityWidget->GetCurrentSettings();
	}

	// Lưu vào biến tạm để apply
	PendingSettings = CurrentSettings;

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Collected all settings from widgets"));
}

void UMainMenuSettingWidget::AsyncApplySettings()
{
	if (bIsApplyingSettings)
		return;

	bIsApplyingSettings = true;

	// Validate trước khi apply
	if (!ValidateAllSettings())
	{
		bIsApplyingSettings = false;
		ShowValidationErrors(ValidationErrors);
		return;
	}

	// Apply settings thông qua subsystem với settings thực tế
	if (SettingsSubsystem)
	{
		// Apply settings đã collect từ các widget
		bool bSuccess = SettingsSubsystem->ApplyAllSettings(PendingSettings);
		OnSettingsApplied_Internal(bSuccess);
	}
	else
	{
		OnSettingsApplied_Internal(false);
	}
}

void UMainMenuSettingWidget::OnSettingsApplied_Internal(bool bSuccess)
{
	bIsApplyingSettings = false;

	if (bSuccess)
	{
		if (SettingsSubsystem)
		{
			// Lưu vào file
			SettingsSubsystem->SaveAllSettings();
		}

		ClearUnsavedChanges();
		PlayApplySuccessAnimation();
		OnSettingsApplied.Broadcast();

		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Settings applied and saved successfully"));

		// Hiển thị thông báo thành công
		ShowSuccessNotification(TEXT("Settings saved successfully!"));
	}
	else
	{
		PlayUISound("Error");
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Failed to apply settings"));

		// Hiển thị thông báo lỗi
		ShowErrorNotification(TEXT("Failed to apply settings. Please try again."));
	}
}

// ===== AUTO-SAVE =====

void UMainMenuSettingWidget::StartAutoSaveTimer()
{
	StopAutoSaveTimer();

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			AutoSaveTimerHandle,
			this,
			&UMainMenuSettingWidget::OnAutoSaveTimerElapsed,
			AutoSaveDelay,
			false
		);
	}
}

void UMainMenuSettingWidget::StopAutoSaveTimer()
{
	UWorld* World = GetWorld();
	if (World && AutoSaveTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
		AutoSaveTimerHandle.Invalidate();
	}
}

void UMainMenuSettingWidget::OnAutoSaveTimerElapsed()
{
	if (bHasUnsavedChanges && !bIsApplyingSettings)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Auto-saving settings"));
		AsyncApplySettings();
	}
}

// ===== TOOLTIPS & HELP =====

void UMainMenuSettingWidget::ShowSettingsHelp(ESettingsCategory Category)
{
	// Show help dialog for specific category
	FString HelpText;

	switch (Category)
	{
	case ESettingsCategory::Gameplay:
		HelpText = TEXT("Gameplay settings affect how the game plays and feels.");
		break;
	case ESettingsCategory::Graphics:
		HelpText = TEXT("Graphics settings affect visual quality and performance.");
		break;
	case ESettingsCategory::Audio:
		HelpText = TEXT("Audio settings control volume levels and sound options.");
		break;
	case ESettingsCategory::Controls:
		HelpText = TEXT("Controls settings let you customize input mappings.");
		break;
	case ESettingsCategory::Accessibility:
		HelpText = TEXT("Accessibility settings help make the game more accessible.");
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Help - %s"), *HelpText);
}

FText UMainMenuSettingWidget::GetSettingTooltip(const FString& SettingName) const
{
	// Return tooltip text for specific setting
	// This should be loaded from a data table or localization
	return FText::FromString(FString::Printf(TEXT("Tooltip for %s"), *SettingName));
}

// ===== PROFILES =====

void UMainMenuSettingWidget::SaveCustomProfile(const FString& ProfileName)
{
	if (!SettingsSubsystem || ProfileName.IsEmpty())
		return;

	// Save current settings as a custom profile
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Saving profile '%s'"), *ProfileName);
}

void UMainMenuSettingWidget::LoadCustomProfile(const FString& ProfileName)
{
	if (!SettingsSubsystem || ProfileName.IsEmpty())
		return;

	// Load settings from custom profile
	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Loading profile '%s'"), *ProfileName);
	RefreshAllWidgets();
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
	ESettingsCategory CurrentCategory = static_cast<ESettingsCategory>(CurrentCategoryIndex);
	FString AnnouncementText = FString::Printf(
		TEXT("Switched to %s settings"),
		*GetCategoryName(CurrentCategory).ToString()
	);

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: %s"), *AnnouncementText);

	// Implement screen reader announcement if available
}

void UMainMenuSettingWidget::AnnounceSettingChange(const FString& SettingName, const FString& NewValue)
{
	FString AnnouncementText = FString::Printf(
		TEXT("%s changed to %s"),
		*SettingName,
		*NewValue
	);

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: %s"), *AnnouncementText);

	// Implement screen reader announcement if available
}

// ===== PUBLIC FUNCTIONS =====

void UMainMenuSettingWidget::RefreshAllWidgets()
{
	if (!SettingsSubsystem)
		return;

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

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: All widgets refreshed with current settings"));
}

void UMainMenuSettingWidget::ForceApplySettings()
{
	if (SettingsSubsystem)
	{
		AsyncApplySettings();
	}
}