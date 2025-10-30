// Fill out your copyright notice in the Description page of Project Settings.

#include "AccessibilityWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Settings/Core/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

UAccessibilityWidget::UAccessibilityWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAccessibilityWidget::NativeConstruct()
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
		UE_LOG(LogTemp, Error, TEXT("AccessibilityWidget: Failed to get SettingsSubsystem"));
		return;
	}

	// Initialize all selections
	InitializeSelections();

	// Load current settings from subsystem
	LoadCurrentSettings();

	// Bind Reset Button
	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UAccessibilityWidget::OnResetButtonClicked);
	}
}

void UAccessibilityWidget::NativeDestruct()
{
	// Unbind all delegates
	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UAccessibilityWidget::OnResetButtonClicked);
	}

	Super::NativeDestruct();
}

void UAccessibilityWidget::InitializeSelections()
{
	// Text Size
	if (TextSizeSelection)
	{
		TextSizeSelection->Clear();
		TextSizeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Small")) });
		TextSizeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Medium")) });
		TextSizeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Large")) });
		TextSizeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Extra Large")) });
		TextSizeSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnTextSizeChanged);
	}

	// Text Contrast
	if (TextContrastSelection)
	{
		TextContrastSelection->Clear();
		TextContrastSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Low")) });
		TextContrastSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Medium")) });
		TextContrastSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("High")) });
		TextContrastSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnTextContrastChanged);
	}

	// Dyslexia Font
	if (DyslexiaFontSelection)
	{
		DyslexiaFontSelection->Clear();
		AddToggleOptions(DyslexiaFontSelection);
		DyslexiaFontSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnDyslexiaFontChanged);
	}

	// Color Blind Mode
	if (ColorBlindModeSelection)
	{
		ColorBlindModeSelection->Clear();
		ColorBlindModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("None")) });
		ColorBlindModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Protanopia")) });
		ColorBlindModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Deuteranopia")) });
		ColorBlindModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Tritanopia")) });
		ColorBlindModeSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnColorBlindModeChanged);
	}

	// High Contrast UI
	if (HighContrastUISelection)
	{
		HighContrastUISelection->Clear();
		AddToggleOptions(HighContrastUISelection);
		HighContrastUISelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnHighContrastUIChanged);
	}

	// Reduced Motion
	if (ReducedMotionSelection)
	{
		ReducedMotionSelection->Clear();
		AddToggleOptions(ReducedMotionSelection);
		ReducedMotionSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnReducedMotionChanged);
	}

	// Photosensitivity Mode
	if (PhotosensitivityModeSelection)
	{
		PhotosensitivityModeSelection->Clear();
		PhotosensitivityModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("None")) });
		PhotosensitivityModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Reduced")) });
		PhotosensitivityModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Maximum")) });
		PhotosensitivityModeSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnPhotosensitivityModeChanged);
	}

	// Screen Reader
	if (ScreenReaderSelection)
	{
		ScreenReaderSelection->Clear();
		AddToggleOptions(ScreenReaderSelection);
		ScreenReaderSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnScreenReaderChanged);
	}

	// Sound Cues Visualization
	if (SoundCuesVisualizationSelection)
	{
		SoundCuesVisualizationSelection->Clear();
		AddToggleOptions(SoundCuesVisualizationSelection);
		SoundCuesVisualizationSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnSoundCuesVisualizationChanged);
	}

	// Haptic Feedback
	if (HapticFeedbackSelection)
	{
		HapticFeedbackSelection->Clear();
		AddToggleOptions(HapticFeedbackSelection);
		HapticFeedbackSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnHapticFeedbackChanged);
	}

	// Single Handed Mode
	if (SingleHandedModeSelection)
	{
		SingleHandedModeSelection->Clear();
		SingleHandedModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("None")) });
		SingleHandedModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Left Hand")) });
		SingleHandedModeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Right Hand")) });
		SingleHandedModeSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnSingleHandedModeChanged);
	}

	// Hold to Activate
	if (HoldToActivateSelection)
	{
		HoldToActivateSelection->Clear();
		AddToggleOptions(HoldToActivateSelection);
		HoldToActivateSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnHoldToActivateChanged);
	}

	// Hold Activation Time (0.1s to 5.0s)
	if (HoldActivationTimeSelection)
	{
		HoldActivationTimeSelection->Clear();
		HoldActivationTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("0.1s")) });
		HoldActivationTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("0.5s")) });
		HoldActivationTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("1.0s")) });
		HoldActivationTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("2.0s")) });
		HoldActivationTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("3.0s")) });
		HoldActivationTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("5.0s")) });
		HoldActivationTimeSelection->OnSelectionChanged.BindDynamic(this, &UAccessibilityWidget::OnHoldActivationTimeChanged);
	}
}

void UAccessibilityWidget::LoadCurrentSettings()
{
	if (!SettingsSubsystem)
		return;

	// Lấy settings từ subsystem và lưu vào CurrentSettings
	CurrentSettings = SettingsSubsystem->GetAllSettings().AccessibilitySettings;

	// Load settings vào UI
	LoadSettings(CurrentSettings);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Loaded current settings from subsystem"));
}

// ===== PUBLIC API =====

void UAccessibilityWidget::LoadSettings(const FS_AccessibilitySettings& Settings)
{
	// Lưu vào CurrentSettings
	CurrentSettings = Settings;

	// Load vào UI (không trigger callbacks)
	bIsLoadingSettings = true;

	// Text Size
	if (TextSizeSelection)
	{
		int32 TextSizeIndex = static_cast<int32>(Settings.TextSize);
		TextSizeSelection->SetCurrentSelection(TextSizeIndex);
	}

	// Text Contrast
	if (TextContrastSelection)
	{
		int32 Index = static_cast<int32>(Settings.TextContrast);
		TextContrastSelection->SetCurrentSelection(Index);
	}

	// Dyslexia Font
	if (DyslexiaFontSelection)
	{
		DyslexiaFontSelection->SetCurrentSelection(Settings.bDyslexiaFontEnabled ? 1 : 0);
	}

	// Color Blind Mode
	if (ColorBlindModeSelection)
	{
		int32 Index = static_cast<int32>(Settings.ColorBlindMode);
		ColorBlindModeSelection->SetCurrentSelection(Index);
	}

	// High Contrast UI
	if (HighContrastUISelection)
	{
		HighContrastUISelection->SetCurrentSelection(Settings.bHighContrastUIEnabled ? 1 : 0);
	}

	// Reduced Motion
	if (ReducedMotionSelection)
	{
		ReducedMotionSelection->SetCurrentSelection(Settings.bReducedMotionEnabled ? 1 : 0);
	}

	// Photosensitivity Mode
	if (PhotosensitivityModeSelection)
	{
		int32 Index = static_cast<int32>(Settings.PhotosensitivityMode);
		PhotosensitivityModeSelection->SetCurrentSelection(Index);
	}

	// Screen Reader
	if (ScreenReaderSelection)
	{
		ScreenReaderSelection->SetCurrentSelection(Settings.bScreenReaderEnabled ? 1 : 0);
	}

	// Sound Cues Visualization
	if (SoundCuesVisualizationSelection)
	{
		SoundCuesVisualizationSelection->SetCurrentSelection(Settings.bSoundCuesVisualizationEnabled ? 1 : 0);
	}

	// Haptic Feedback
	if (HapticFeedbackSelection)
	{
		HapticFeedbackSelection->SetCurrentSelection(Settings.bHapticFeedbackEnabled ? 1 : 0);
	}

	// Single Handed Mode
	if (SingleHandedModeSelection)
	{
		int32 Index = static_cast<int32>(Settings.SingleHandedMode);
		SingleHandedModeSelection->SetCurrentSelection(Index);
	}

	// Hold to Activate
	if (HoldToActivateSelection)
	{
		HoldToActivateSelection->SetCurrentSelection(Settings.bEnableHoldToActivate ? 1 : 0);
	}

	// Hold Activation Time
	if (HoldActivationTimeSelection)
	{
		int32 Index = ActivationTimeToIndex(Settings.HoldActivationTime);
		HoldActivationTimeSelection->SetCurrentSelection(Index);
	}

	bIsLoadingSettings = false;

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Settings loaded into UI"));
}

FS_AccessibilitySettings UAccessibilityWidget::GetCurrentSettings() const
{
	return CurrentSettings;
}

TArray<FString> UAccessibilityWidget::ValidateSettings() const
{
	TArray<FString> Errors;

	// Validate Hold Activation Time
	if (CurrentSettings.HoldActivationTime < 0.0f || CurrentSettings.HoldActivationTime > 5.0f)
	{
		Errors.Add(TEXT("Hold Activation Time must be between 0.0 and 5.0 seconds"));
	}

	if (Errors.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AccessibilityWidget: Validation found %d errors"), Errors.Num());
	}

	return Errors;
}

// ===== CALLBACKS =====
// CHỈ cập nhật CurrentSettings, KHÔNG gọi subsystem

void UAccessibilityWidget::OnTextSizeChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_TextSize Size = static_cast<EE_TextSize>(NewIndex);
	CurrentSettings.TextSize = Size;

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Text size changed to index %d (not saved yet)"), NewIndex);
}

void UAccessibilityWidget::OnTextContrastChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_TextContrast Contrast = static_cast<EE_TextContrast>(NewIndex);
	CurrentSettings.TextContrast = Contrast;

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Text contrast changed to index %d (not saved yet)"), NewIndex);
}

void UAccessibilityWidget::OnDyslexiaFontChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bDyslexiaFontEnabled = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Dyslexia font %s (not saved yet)"),
		CurrentSettings.bDyslexiaFontEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAccessibilityWidget::OnColorBlindModeChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_ColorBlindMode Mode = static_cast<EE_ColorBlindMode>(NewIndex);
	CurrentSettings.ColorBlindMode = Mode;

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Color blind mode changed to index %d (not saved yet)"), NewIndex);
}

void UAccessibilityWidget::OnHighContrastUIChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bHighContrastUIEnabled = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: High contrast UI %s (not saved yet)"),
		CurrentSettings.bHighContrastUIEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAccessibilityWidget::OnReducedMotionChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bReducedMotionEnabled = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Reduced motion %s (not saved yet)"),
		CurrentSettings.bReducedMotionEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAccessibilityWidget::OnPhotosensitivityModeChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_PhotosensitivityMode Mode = static_cast<EE_PhotosensitivityMode>(NewIndex);
	CurrentSettings.PhotosensitivityMode = Mode;

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Photosensitivity mode changed to index %d (not saved yet)"), NewIndex);
}

void UAccessibilityWidget::OnScreenReaderChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bScreenReaderEnabled = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Screen reader %s (not saved yet)"),
		CurrentSettings.bScreenReaderEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAccessibilityWidget::OnSoundCuesVisualizationChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bSoundCuesVisualizationEnabled = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Sound cues visualization %s (not saved yet)"),
		CurrentSettings.bSoundCuesVisualizationEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAccessibilityWidget::OnHapticFeedbackChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bHapticFeedbackEnabled = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Haptic feedback %s (not saved yet)"),
		CurrentSettings.bHapticFeedbackEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAccessibilityWidget::OnSingleHandedModeChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_SingleHandedMode Mode = static_cast<EE_SingleHandedMode>(NewIndex);
	CurrentSettings.SingleHandedMode = Mode;

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Single handed mode changed to index %d (not saved yet)"), NewIndex);
}

void UAccessibilityWidget::OnHoldToActivateChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bEnableHoldToActivate = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Hold to activate %s (not saved yet)"),
		CurrentSettings.bEnableHoldToActivate ? TEXT("enabled") : TEXT("disabled"));
}

void UAccessibilityWidget::OnHoldActivationTimeChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	float Time = IndexToActivationTime(NewIndex);
	CurrentSettings.HoldActivationTime = Time;

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Hold activation time changed to %.1fs (not saved yet)"), Time);
}

void UAccessibilityWidget::OnResetButtonClicked()
{
	// Reset về default settings
	FS_AccessibilitySettings DefaultSettings;
	LoadSettings(DefaultSettings);

	UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: Reset to default settings (not saved yet)"));
}

// ===== HELPER FUNCTIONS =====

void UAccessibilityWidget::AddToggleOptions(USelectionWidget* Selection)
{
	if (Selection)
	{
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("Off")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("On")) });
	}
}

float UAccessibilityWidget::IndexToActivationTime(int32 Index)
{
	TArray<float> Times = { 0.1f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f };
	if (Index >= 0 && Index < Times.Num())
	{
		return Times[Index];
	}
	return 1.0f; // Default
}

int32 UAccessibilityWidget::ActivationTimeToIndex(float Value)
{
	TArray<float> Times = { 0.1f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f };

	// Find closest match
	int32 ClosestIndex = 2; // Default to 1.0s
	float ClosestDiff = FLT_MAX;

	for (int32 i = 0; i < Times.Num(); i++)
	{
		float Diff = FMath::Abs(Value - Times[i]);
		if (Diff < ClosestDiff)
		{
			ClosestDiff = Diff;
			ClosestIndex = i;
		}
	}

	return ClosestIndex;
}