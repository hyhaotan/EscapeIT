// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Settings/Tab/AccessibilityWidget.h"
#include "UI/Settings/Row/SelectionSettingRow.h"
#include "Settings/Core/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

UAccessibilityWidget::UAccessibilityWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsLoadingSettings(false)
    , SettingsSubsystem(nullptr)
{
}

void UAccessibilityWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get Settings Subsystem (optional)
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
    }

    if (!SettingsSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityWidget: SettingsSubsystem not found; widget will still work locally"));
    }

    // Initialize selection rows and bind their delegates
    InitializeSelectionRows();

    // Load current settings either from subsystem (if available) or defaults
    if (SettingsSubsystem)
    {
        LoadSettings(SettingsSubsystem->GetAllSettings().AccessibilitySettings);
    }
    else
    {
        FS_AccessibilitySettings DefaultSettings;
        LoadSettings(DefaultSettings);
    }
}

void UAccessibilityWidget::NativeDestruct()
{
    // Unbind all selection row delegates
    if (TextSizeRow)
        TextSizeRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnTextSizeChanged);
    if (TextContrastRow)
        TextContrastRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnTextContrastChanged);
    if (DyslexiaFontRow)
        DyslexiaFontRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnDyslexiaFontChanged);
    if (ColorBlindModeRow)
        ColorBlindModeRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnColorBlindModeChanged);
    if (HighContrastUIRow)
        HighContrastUIRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnHighContrastUIChanged);
    if (ReducedMotionRow)
        ReducedMotionRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnReducedMotionChanged);
    if (PhotosensitivityModeRow)
        PhotosensitivityModeRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnPhotosensitivityModeChanged);
    if (ScreenReaderRow)
        ScreenReaderRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnScreenReaderChanged);
    if (SoundCuesVisualizationRow)
        SoundCuesVisualizationRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnSoundCuesVisualizationChanged);
    if (HapticFeedbackRow)
        HapticFeedbackRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnHapticFeedbackChanged);
    if (SingleHandedModeRow)
        SingleHandedModeRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnSingleHandedModeChanged);
    if (HoldToActivateRow)
        HoldToActivateRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnHoldToActivateChanged);
    if (HoldActivationTimeRow)
        HoldActivationTimeRow->OnSelectionChanged.RemoveDynamic(this, &UAccessibilityWidget::OnHoldActivationTimeChanged);

    Super::NativeDestruct();
}

void UAccessibilityWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

// Initialize selection rows with options and bind delegates
void UAccessibilityWidget::InitializeSelectionRows()
{
    const TArray<FText> ToggleOptions = MakeToggleOptions();
    const TArray<FText> TextSizeOptions = MakeTextSizeOptions();
    const TArray<FText> TextContrastOptions = MakeTextContrastOptions();
    const TArray<FText> ColorBlindModeOptions = MakeColorBlindModeOptions();
    const TArray<FText> PhotosensitivityModeOptions = MakePhotosensitivityModeOptions();
    const TArray<FText> SingleHandedModeOptions = MakeSingleHandedModeOptions();
    const TArray<FText> HoldActivationTimeOptions = MakeHoldActivationTimeOptions();

    if (TextSizeRow)
    {
        TextSizeRow->InitializeRow(TextSizeOptions, static_cast<int32>(CurrentSettings.TextSize), FText::FromString(TEXT("Text Size")));
        TextSizeRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnTextSizeChanged);
    }

    if (TextContrastRow)
    {
        TextContrastRow->InitializeRow(TextContrastOptions, static_cast<int32>(CurrentSettings.TextContrast), FText::FromString(TEXT("Text Contrast")));
        TextContrastRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnTextContrastChanged);
    }

    if (DyslexiaFontRow)
    {
        DyslexiaFontRow->InitializeRow(ToggleOptions, CurrentSettings.bDyslexiaFontEnabled ? 1 : 0, FText::FromString(TEXT("Dyslexia Font")));
        DyslexiaFontRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnDyslexiaFontChanged);
    }

    if (ColorBlindModeRow)
    {
        ColorBlindModeRow->InitializeRow(ColorBlindModeOptions, static_cast<int32>(CurrentSettings.ColorBlindMode), FText::FromString(TEXT("Color Blind Mode")));
        ColorBlindModeRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnColorBlindModeChanged);
    }

    if (HighContrastUIRow)
    {
        HighContrastUIRow->InitializeRow(ToggleOptions, CurrentSettings.bHighContrastUIEnabled ? 1 : 0, FText::FromString(TEXT("High Contrast UI")));
        HighContrastUIRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnHighContrastUIChanged);
    }

    if (ReducedMotionRow)
    {
        ReducedMotionRow->InitializeRow(ToggleOptions, CurrentSettings.bReducedMotionEnabled ? 1 : 0, FText::FromString(TEXT("Reduced Motion")));
        ReducedMotionRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnReducedMotionChanged);
    }

    if (PhotosensitivityModeRow)
    {
        PhotosensitivityModeRow->InitializeRow(PhotosensitivityModeOptions, static_cast<int32>(CurrentSettings.PhotosensitivityMode), FText::FromString(TEXT("Photosensitivity Mode")));
        PhotosensitivityModeRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnPhotosensitivityModeChanged);
    }

    if (ScreenReaderRow)
    {
        ScreenReaderRow->InitializeRow(ToggleOptions, CurrentSettings.bScreenReaderEnabled ? 1 : 0, FText::FromString(TEXT("Screen Reader")));
        ScreenReaderRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnScreenReaderChanged);
    }

    if (SoundCuesVisualizationRow)
    {
        SoundCuesVisualizationRow->InitializeRow(ToggleOptions, CurrentSettings.bSoundCuesVisualizationEnabled ? 1 : 0, FText::FromString(TEXT("Sound Cues Visualization")));
        SoundCuesVisualizationRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnSoundCuesVisualizationChanged);
    }

    if (HapticFeedbackRow)
    {
        HapticFeedbackRow->InitializeRow(ToggleOptions, CurrentSettings.bHapticFeedbackEnabled ? 1 : 0, FText::FromString(TEXT("Haptic Feedback")));
        HapticFeedbackRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnHapticFeedbackChanged);
    }

    if (SingleHandedModeRow)
    {
        SingleHandedModeRow->InitializeRow(SingleHandedModeOptions, static_cast<int32>(CurrentSettings.SingleHandedMode), FText::FromString(TEXT("Single Handed Mode")));
        SingleHandedModeRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnSingleHandedModeChanged);
    }

    if (HoldToActivateRow)
    {
        HoldToActivateRow->InitializeRow(ToggleOptions, CurrentSettings.bEnableHoldToActivate ? 1 : 0, FText::FromString(TEXT("Hold to Activate")));
        HoldToActivateRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnHoldToActivateChanged);
    }

    if (HoldActivationTimeRow)
    {
        int32 Index = ActivationTimeToIndex(CurrentSettings.HoldActivationTime);
        HoldActivationTimeRow->InitializeRow(HoldActivationTimeOptions, Index, FText::FromString(TEXT("Hold Activation Time")));
        HoldActivationTimeRow->OnSelectionChanged.AddDynamic(this, &UAccessibilityWidget::OnHoldActivationTimeChanged);
    }
}

TArray<FText> UAccessibilityWidget::MakeToggleOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Off")));
    Out.Add(FText::FromString(TEXT("On")));
    return Out;
}

TArray<FText> UAccessibilityWidget::MakeTextSizeOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Small")));
    Out.Add(FText::FromString(TEXT("Medium")));
    Out.Add(FText::FromString(TEXT("Large")));
    Out.Add(FText::FromString(TEXT("Extra Large")));
    return Out;
}

TArray<FText> UAccessibilityWidget::MakeTextContrastOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Normal")));
    Out.Add(FText::FromString(TEXT("Medium")));
    Out.Add(FText::FromString(TEXT("High")));
    return Out;
}

TArray<FText> UAccessibilityWidget::MakeColorBlindModeOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("None")));
    Out.Add(FText::FromString(TEXT("Protanopia")));
    Out.Add(FText::FromString(TEXT("Deuteranopia")));
    Out.Add(FText::FromString(TEXT("Tritanopia")));
    return Out;
}

TArray<FText> UAccessibilityWidget::MakePhotosensitivityModeOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Off")));
    Out.Add(FText::FromString(TEXT("Reduce Flashing")));
    Out.Add(FText::FromString(TEXT("Remove Flashing")));
    return Out;
}

TArray<FText> UAccessibilityWidget::MakeSingleHandedModeOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Off")));
    Out.Add(FText::FromString(TEXT("Left Hand")));
    Out.Add(FText::FromString(TEXT("Right Hand")));
    return Out;
}

TArray<FText> UAccessibilityWidget::MakeHoldActivationTimeOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("0.1s")));
    Out.Add(FText::FromString(TEXT("0.5s")));
    Out.Add(FText::FromString(TEXT("1.0s")));
    Out.Add(FText::FromString(TEXT("2.0s")));
    Out.Add(FText::FromString(TEXT("3.0s")));
    Out.Add(FText::FromString(TEXT("5.0s")));
    return Out;
}

void UAccessibilityWidget::LoadSettings(const FS_AccessibilitySettings& Settings)
{
    bIsLoadingSettings = true;
    CurrentSettings = Settings;

    // Update selection rows (do not trigger delegates)
    if (TextSizeRow)
        TextSizeRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.TextSize), false);

    if (TextContrastRow)
        TextContrastRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.TextContrast), false);

    if (DyslexiaFontRow)
        DyslexiaFontRow->SetCurrentSelection(CurrentSettings.bDyslexiaFontEnabled ? 1 : 0, false);

    if (ColorBlindModeRow)
        ColorBlindModeRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.ColorBlindMode), false);

    if (HighContrastUIRow)
        HighContrastUIRow->SetCurrentSelection(CurrentSettings.bHighContrastUIEnabled ? 1 : 0, false);

    if (ReducedMotionRow)
        ReducedMotionRow->SetCurrentSelection(CurrentSettings.bReducedMotionEnabled ? 1 : 0, false);

    if (PhotosensitivityModeRow)
        PhotosensitivityModeRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.PhotosensitivityMode), false);

    if (ScreenReaderRow)
        ScreenReaderRow->SetCurrentSelection(CurrentSettings.bScreenReaderEnabled ? 1 : 0, false);

    if (SoundCuesVisualizationRow)
        SoundCuesVisualizationRow->SetCurrentSelection(CurrentSettings.bSoundCuesVisualizationEnabled ? 1 : 0, false);

    if (HapticFeedbackRow)
        HapticFeedbackRow->SetCurrentSelection(CurrentSettings.bHapticFeedbackEnabled ? 1 : 0, false);

    if (SingleHandedModeRow)
        SingleHandedModeRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.SingleHandedMode), false);

    if (HoldToActivateRow)
        HoldToActivateRow->SetCurrentSelection(CurrentSettings.bEnableHoldToActivate ? 1 : 0, false);

    if (HoldActivationTimeRow)
    {
        int32 Index = ActivationTimeToIndex(CurrentSettings.HoldActivationTime);
        HoldActivationTimeRow->SetCurrentSelection(Index, false);
    }

    bIsLoadingSettings = false;
}

FS_AccessibilitySettings UAccessibilityWidget::GetCurrentSettings() const
{
    return CurrentSettings;
}

TArray<FString> UAccessibilityWidget::ValidateSettings() const
{
    TArray<FString> Errors;

    if (CurrentSettings.HoldActivationTime < 0.0f || CurrentSettings.HoldActivationTime > 5.0f)
    {
        Errors.Add(TEXT("Hold Activation Time out of range (0.0 - 5.0)"));
    }

    if (Errors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityWidget: Validation found %d errors"), Errors.Num());
    }

    return Errors;
}

// Selection callbacks

void UAccessibilityWidget::OnTextSizeChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.TextSize = static_cast<EE_TextSize>(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: TextSize changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnTextContrastChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.TextContrast = static_cast<EE_TextContrast>(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: TextContrast changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnDyslexiaFontChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bDyslexiaFontEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: DyslexiaFont changed to %s (local)"), CurrentSettings.bDyslexiaFontEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnColorBlindModeChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.ColorBlindMode = static_cast<EE_ColorBlindMode>(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: ColorBlindMode changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnHighContrastUIChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bHighContrastUIEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: HighContrastUI changed to %s (local)"), CurrentSettings.bHighContrastUIEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnReducedMotionChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bReducedMotionEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: ReducedMotion changed to %s (local)"), CurrentSettings.bReducedMotionEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnPhotosensitivityModeChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.PhotosensitivityMode = static_cast<EE_PhotosensitivityMode>(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: PhotosensitivityMode changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnScreenReaderChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bScreenReaderEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: ScreenReader changed to %s (local)"), CurrentSettings.bScreenReaderEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnSoundCuesVisualizationChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bSoundCuesVisualizationEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: SoundCuesVisualization changed to %s (local)"), CurrentSettings.bSoundCuesVisualizationEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnHapticFeedbackChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bHapticFeedbackEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: HapticFeedback changed to %s (local)"), CurrentSettings.bHapticFeedbackEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnSingleHandedModeChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.SingleHandedMode = static_cast<EE_SingleHandedMode>(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: SingleHandedMode changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnHoldToActivateChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bEnableHoldToActivate = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: HoldToActivate changed to %s (local)"), CurrentSettings.bEnableHoldToActivate ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

void UAccessibilityWidget::OnHoldActivationTimeChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    float Time = IndexToActivationTime(NewIndex);
    CurrentSettings.HoldActivationTime = Time;
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: HoldActivationTime updated -> %.1fs"), Time);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAccessibilitySettings(CurrentSettings);
}

// Helper functions

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