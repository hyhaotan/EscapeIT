// Fill out your copyright notice in the Description page of Project Settings.

#include "AccessibilityWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Subsystem/SettingsSubsystem.h"
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

	// Load current settings
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

	FS_AccessibilitySettings CurrentSettings = SettingsSubsystem->GetAllSettings().AccessibilitySettings;

	// Text Size
	if (TextSizeSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.TextSize);
		TextSizeSelection->SetCurrentSelection(Index);
	}

	// Text Contrast
	if (TextContrastSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.TextContrast);
		TextContrastSelection->SetCurrentSelection(Index);
	}

	// Dyslexia Font
	if (DyslexiaFontSelection)
	{
		DyslexiaFontSelection->SetCurrentSelection(CurrentSettings.bDyslexiaFontEnabled ? 1 : 0);
	}

	// Color Blind Mode
	if (ColorBlindModeSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.ColorBlindMode);
		ColorBlindModeSelection->SetCurrentSelection(Index);
	}

	// High Contrast UI
	if (HighContrastUISelection)
	{
		HighContrastUISelection->SetCurrentSelection(CurrentSettings.bHighContrastUIEnabled ? 1 : 0);
	}

	// Reduced Motion
	if (ReducedMotionSelection)
	{
		ReducedMotionSelection->SetCurrentSelection(CurrentSettings.bReducedMotionEnabled ? 1 : 0);
	}

	// Photosensitivity Mode
	if (PhotosensitivityModeSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.PhotosensitivityMode);
		PhotosensitivityModeSelection->SetCurrentSelection(Index);
	}

	// Screen Reader
	if (ScreenReaderSelection)
	{
		ScreenReaderSelection->SetCurrentSelection(CurrentSettings.bScreenReaderEnabled ? 1 : 0);
	}

	// Sound Cues Visualization
	if (SoundCuesVisualizationSelection)
	{
		SoundCuesVisualizationSelection->SetCurrentSelection(CurrentSettings.bSoundCuesVisualizationEnabled ? 1 : 0);
	}

	// Haptic Feedback
	if (HapticFeedbackSelection)
	{
		HapticFeedbackSelection->SetCurrentSelection(CurrentSettings.bHapticFeedbackEnabled ? 1 : 0);
	}

	// Single Handed Mode
	if (SingleHandedModeSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.SingleHandedMode);
		SingleHandedModeSelection->SetCurrentSelection(Index);
	}

	// Hold to Activate
	if (HoldToActivateSelection)
	{
		HoldToActivateSelection->SetCurrentSelection(CurrentSettings.bEnableHoldToActivate ? 1 : 0);
	}

	// Hold Activation Time
	if (HoldActivationTimeSelection)
	{
		int32 Index = ActivationTimeToIndex(CurrentSettings.HoldActivationTime);
		HoldActivationTimeSelection->SetCurrentSelection(Index);
	}
}

// ===== CALLBACKS =====

void UAccessibilityWidget::OnTextSizeChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_TextSize Size = static_cast<EE_TextSize>(NewIndex);
		SettingsSubsystem->SetTextSize(Size);
	}
}

void UAccessibilityWidget::OnTextContrastChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_TextContrast Contrast = static_cast<EE_TextContrast>(NewIndex);
		SettingsSubsystem->SetTextContrast(Contrast);
	}
}

void UAccessibilityWidget::OnDyslexiaFontChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetDyslexiaFontEnabled(NewIndex == 1);
	}
}

void UAccessibilityWidget::OnColorBlindModeChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_ColorBlindMode Mode = static_cast<EE_ColorBlindMode>(NewIndex);
		SettingsSubsystem->SetColorBlindMode(Mode);
	}
}

void UAccessibilityWidget::OnHighContrastUIChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetHighContrastUIEnabled(NewIndex == 1);
	}
}

void UAccessibilityWidget::OnReducedMotionChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetReducedMotionEnabled(NewIndex == 1);
	}
}

void UAccessibilityWidget::OnPhotosensitivityModeChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_PhotosensitivityMode Mode = static_cast<EE_PhotosensitivityMode>(NewIndex);
		SettingsSubsystem->SetPhotosensitivityMode(Mode);
	}
}

void UAccessibilityWidget::OnScreenReaderChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetScreenReaderEnabled(NewIndex == 1);
	}
}

void UAccessibilityWidget::OnSoundCuesVisualizationChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetSoundCuesVisualizationEnabled(NewIndex == 1);
	}
}

void UAccessibilityWidget::OnHapticFeedbackChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetHapticFeedbackEnabled(NewIndex == 1);
	}
}

void UAccessibilityWidget::OnSingleHandedModeChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_SingleHandedMode Mode = static_cast<EE_SingleHandedMode>(NewIndex);
		SettingsSubsystem->SetSingleHandedMode(Mode);
	}
}

void UAccessibilityWidget::OnHoldToActivateChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetEnableHoldToActivate(NewIndex == 1);
	}
}

void UAccessibilityWidget::OnHoldActivationTimeChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		float Time = IndexToActivationTime(NewIndex);
		SettingsSubsystem->SetHoldActivationTime(Time);
	}
}

void UAccessibilityWidget::OnResetButtonClicked()
{
	if (SettingsSubsystem)
	{
		// Reset only accessibility settings to default
		FS_AccessibilitySettings DefaultSettings;
		SettingsSubsystem->ApplyAccessibilitySettings(DefaultSettings);

		// Reload UI
		LoadCurrentSettings();
	}
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