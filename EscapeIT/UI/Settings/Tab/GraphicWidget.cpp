// GraphicWidget.cpp
#include "GraphicWidget.h"
#include "EscapeIT/UI/Settings/Row/SelectionSettingRow.h"
#include "EscapeIT/Settings/Core/SettingsSubsystem.h"
#include "CommonTextBlock.h"
#include "IMediaControls.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine.h"

UGraphicWidget::UGraphicWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsLoadingSettings(false)
    , SettingsSubsystem(nullptr)
{
}

void UGraphicWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get Settings Subsystem (optional)
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
    }

    if (!SettingsSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphicWidget: SettingsSubsystem not found; widget will still work locally"));
    }

    // Initialize selection rows and bind their delegates
    InitializeSelectionRows();

    // Load current settings either from subsystem (if available) or defaults
    if (SettingsSubsystem)
    {
        LoadSettings(SettingsSubsystem->GetAllSettings().GraphicsSettings);
    }
    else
    {
        FS_GraphicsSettings DefaultSettings;
        LoadSettings(DefaultSettings);
    }
}

void UGraphicWidget::NativeDestruct()
{
    // Clear FPS timer
    if (FPSTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(FPSTimerHandle);
        FPSTimerHandle.Invalidate();
    }

    // Unbind selection row delegates
    if (QualityPresetRow)
        QualityPresetRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnQualityPresetChanged);
    if (ResolutionRow)
        ResolutionRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnResolutionChanged);
    if (VSyncRow)
        VSyncRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnVSyncChanged);
    if (FrameRateCapRow)
        FrameRateCapRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnFrameRateCapChanged);
    if (RayTracingRow)
        RayTracingRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnRayTracingChanged);
    if (RayTracingQualityRow)
        RayTracingQualityRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnRayTracingQualityChanged);
    if (ShadowQualityRow)
        ShadowQualityRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnShadowQualityChanged);
    if (TextureQualityRow)
        TextureQualityRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnTextureQualityChanged);
    if (AntiAliasingRow)
        AntiAliasingRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnAntiAliasingChanged);
    if (MotionBlurRow)
        MotionBlurRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnMotionBlurChanged);
    if (FieldOfViewRow)
        FieldOfViewRow->OnSelectionChanged.RemoveDynamic(this, &UGraphicWidget::OnFieldOfViewChanged);

    Super::NativeDestruct();
}

// Initialize selection rows with options and bind delegates
void UGraphicWidget::InitializeSelectionRows()
{
    const TArray<FText> ToggleOptions = MakeToggleOptions();
    const TArray<FText> QualityPresetOptions = MakeQualityPresetOptions();
    const TArray<FText> ResolutionOptions = MakeResolutionOptions();
    const TArray<FText> FrameRateCapOptions = MakeFrameRateCapOptions();
    const TArray<FText> RayTracingQualityOptions = MakeRayTracingQualityOptions();
    const TArray<FText> QualityLevelOptions = MakeQualityLevelOptions();
    const TArray<FText> AntiAliasingOptions = MakeAntiAliasingOptions();
    const TArray<FText> PercentageOptions = MakePercentageOptions();
    const TArray<FText> FieldOfViewOptions = MakeFieldOfViewOptions();

    if (QualityPresetRow)
    {
        QualityPresetRow->InitializeRow(QualityPresetOptions, static_cast<int32>(CurrentSettings.QualityPreset), FText::FromString(TEXT("Quality Preset")));
        QualityPresetRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnQualityPresetChanged);
    }

    if (ResolutionRow)
    {
        FIntPoint CurrentResolution(CurrentSettings.ResolutionX, CurrentSettings.ResolutionY);
        ResolutionRow->InitializeRow(ResolutionOptions, ResolutionToIndex(CurrentResolution), FText::FromString(TEXT("Resolution")));
        ResolutionRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnResolutionChanged);
    }

    if (VSyncRow)
    {
        VSyncRow->InitializeRow(ToggleOptions, CurrentSettings.bVSyncEnabled ? 1 : 0, FText::FromString(TEXT("VSync")));
        VSyncRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnVSyncChanged);
    }

    if (FrameRateCapRow)
    {
        FrameRateCapRow->InitializeRow(FrameRateCapOptions, FrameRateCapToIndex(CurrentSettings.FrameRateCap), FText::FromString(TEXT("Frame Rate Cap")));
        FrameRateCapRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnFrameRateCapChanged);
    }

    if (RayTracingRow)
    {
        RayTracingRow->InitializeRow(ToggleOptions, CurrentSettings.bRayTracingEnabled ? 1 : 0, FText::FromString(TEXT("Ray Tracing")));
        RayTracingRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnRayTracingChanged);
    }

    if (RayTracingQualityRow)
    {
        RayTracingQualityRow->InitializeRow(RayTracingQualityOptions, static_cast<int32>(CurrentSettings.RayTracingQuality), FText::FromString(TEXT("Ray Tracing Quality")));
        RayTracingQualityRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnRayTracingQualityChanged);
        RayTracingQualityRow->SetIsEnabled(CurrentSettings.bRayTracingEnabled);
    }

    if (ShadowQualityRow)
    {
        ShadowQualityRow->InitializeRow(QualityLevelOptions, static_cast<int32>(CurrentSettings.ShadowQuality), FText::FromString(TEXT("Shadow Quality")));
        ShadowQualityRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnShadowQualityChanged);
    }

    if (TextureQualityRow)
    {
        TextureQualityRow->InitializeRow(QualityLevelOptions, static_cast<int32>(CurrentSettings.TextureQuality), FText::FromString(TEXT("Texture Quality")));
        TextureQualityRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnTextureQualityChanged);
    }

    if (AntiAliasingRow)
    {
        AntiAliasingRow->InitializeRow(AntiAliasingOptions, static_cast<int32>(CurrentSettings.AntiAliasingMethod), FText::FromString(TEXT("Anti-Aliasing")));
        AntiAliasingRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnAntiAliasingChanged);
    }

    if (MotionBlurRow)
    {
        MotionBlurRow->InitializeRow(PercentageOptions, PercentageToIndex(CurrentSettings.MotionBlurAmount), FText::FromString(TEXT("Motion Blur")));
        MotionBlurRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnMotionBlurChanged);
    }

    if (FieldOfViewRow)
    {
        FieldOfViewRow->InitializeRow(FieldOfViewOptions, FOVToIndex(CurrentSettings.FieldOfView), FText::FromString(TEXT("Field of View")));
        FieldOfViewRow->OnSelectionChanged.AddDynamic(this, &UGraphicWidget::OnFieldOfViewChanged);
    }
}

TArray<FText> UGraphicWidget::MakeToggleOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Off")));
    Out.Add(FText::FromString(TEXT("On")));
    return Out;
}

TArray<FText> UGraphicWidget::MakeQualityPresetOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Low")));
    Out.Add(FText::FromString(TEXT("Medium")));
    Out.Add(FText::FromString(TEXT("High")));
    Out.Add(FText::FromString(TEXT("Ultra")));
    Out.Add(FText::FromString(TEXT("Custom")));
    return Out;
}

TArray<FText> UGraphicWidget::MakeResolutionOptions() const
{
    TArray<FText> Out;
    TArray<FIntPoint> Resolutions = GetAvailableResolutions();
    for (const FIntPoint& Res : Resolutions)
    {
        FString ResText = FString::Printf(TEXT("%dx%d"), Res.X, Res.Y);
        Out.Add(FText::FromString(ResText));
    }
    return Out;
}

TArray<FText> UGraphicWidget::MakeFrameRateCapOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("30 FPS")));
    Out.Add(FText::FromString(TEXT("60 FPS")));
    Out.Add(FText::FromString(TEXT("90 FPS")));
    Out.Add(FText::FromString(TEXT("120 FPS")));
    Out.Add(FText::FromString(TEXT("144 FPS")));
    Out.Add(FText::FromString(TEXT("Unlimited")));
    return Out;
}

TArray<FText> UGraphicWidget::MakeRayTracingQualityOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Low")));
    Out.Add(FText::FromString(TEXT("Medium")));
    Out.Add(FText::FromString(TEXT("High")));
    Out.Add(FText::FromString(TEXT("Epic")));
    return Out;
}

TArray<FText> UGraphicWidget::MakeQualityLevelOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Low")));
    Out.Add(FText::FromString(TEXT("Medium")));
    Out.Add(FText::FromString(TEXT("High")));
    Out.Add(FText::FromString(TEXT("Epic")));
    return Out;
}

TArray<FText> UGraphicWidget::MakeAntiAliasingOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Off")));
    Out.Add(FText::FromString(TEXT("FXAA")));
    Out.Add(FText::FromString(TEXT("TAA")));
    Out.Add(FText::FromString(TEXT("MSAA x2")));
    Out.Add(FText::FromString(TEXT("MSAA x4")));
    return Out;
}

TArray<FText> UGraphicWidget::MakePercentageOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("0%")));
    Out.Add(FText::FromString(TEXT("25%")));
    Out.Add(FText::FromString(TEXT("50%")));
    Out.Add(FText::FromString(TEXT("75%")));
    Out.Add(FText::FromString(TEXT("100%")));
    return Out;
}

TArray<FText> UGraphicWidget::MakeFieldOfViewOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("70°")));
    Out.Add(FText::FromString(TEXT("80°")));
    Out.Add(FText::FromString(TEXT("90°")));
    Out.Add(FText::FromString(TEXT("100°")));
    Out.Add(FText::FromString(TEXT("110°")));
    Out.Add(FText::FromString(TEXT("120°")));
    return Out;
}

void UGraphicWidget::LoadSettings(const FS_GraphicsSettings& Settings)
{
    bIsLoadingSettings = true;
    CurrentSettings = Settings;

    // Update selection rows (do not trigger delegates)
    if (QualityPresetRow)
        QualityPresetRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.QualityPreset), false);

    if (ResolutionRow)
    {
        FIntPoint Resolution(CurrentSettings.ResolutionX, CurrentSettings.ResolutionY);
        ResolutionRow->SetCurrentSelection(ResolutionToIndex(Resolution), false);
    }

    if (VSyncRow)
        VSyncRow->SetCurrentSelection(CurrentSettings.bVSyncEnabled ? 1 : 0, false);

    if (FrameRateCapRow)
        FrameRateCapRow->SetCurrentSelection(FrameRateCapToIndex(CurrentSettings.FrameRateCap), false);

    if (RayTracingRow)
        RayTracingRow->SetCurrentSelection(CurrentSettings.bRayTracingEnabled ? 1 : 0, false);

    if (RayTracingQualityRow)
    {
        RayTracingQualityRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.RayTracingQuality), false);
        RayTracingQualityRow->SetIsEnabled(CurrentSettings.bRayTracingEnabled);
    }

    if (ShadowQualityRow)
        ShadowQualityRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.ShadowQuality), false);

    if (TextureQualityRow)
        TextureQualityRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.TextureQuality), false);

    if (AntiAliasingRow)
        AntiAliasingRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.AntiAliasingMethod), false);

    if (MotionBlurRow)
        MotionBlurRow->SetCurrentSelection(PercentageToIndex(CurrentSettings.MotionBlurAmount), false);

    if (FieldOfViewRow)
        FieldOfViewRow->SetCurrentSelection(FOVToIndex(CurrentSettings.FieldOfView), false);

    bIsLoadingSettings = false;
}

FS_GraphicsSettings UGraphicWidget::GetCurrentSettings() const
{
    return CurrentSettings;
}

TArray<FString> UGraphicWidget::ValidateSettings() const
{
    TArray<FString> Errors;

    if (CurrentSettings.ResolutionX < 640 || CurrentSettings.ResolutionX > 7680)
    {
        Errors.Add(TEXT("Resolution width out of range (640 - 7680)"));
    }
    if (CurrentSettings.ResolutionY < 480 || CurrentSettings.ResolutionY > 4320)
    {
        Errors.Add(TEXT("Resolution height out of range (480 - 4320)"));
    }
    if (CurrentSettings.FrameRateCap != 0 && (CurrentSettings.FrameRateCap < 30 || CurrentSettings.FrameRateCap > 300))
    {
        Errors.Add(TEXT("Frame rate cap out of range (0 or 30 - 300)"));
    }
    if (CurrentSettings.FieldOfView < 60.0f || CurrentSettings.FieldOfView > 120.0f)
    {
        Errors.Add(TEXT("Field of view out of range (60 - 120)"));
    }
    if (CurrentSettings.MotionBlurAmount < 0.0f || CurrentSettings.MotionBlurAmount > 1.0f)
    {
        Errors.Add(TEXT("Motion blur amount out of range (0.0 - 1.0)"));
    }

    if (CurrentSettings.bRayTracingEnabled)
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphicWidget: Ray tracing enabled - ensure hardware supports it"));
    }

    if (CurrentSettings.QualityPreset == EE_GraphicsQuality::Ultra &&
        CurrentSettings.ResolutionX >= 3840 &&
        CurrentSettings.bRayTracingEnabled)
    {
        Errors.Add(TEXT("Warning: Ultra quality with 4K+ resolution and ray tracing may impact performance"));
    }

    if (Errors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphicWidget: Validation found %d errors"), Errors.Num());
    }

    return Errors;
}

// Conversion helpers

TArray<FIntPoint> UGraphicWidget::GetAvailableResolutions() const
{
    TArray<FIntPoint> Resolutions;

    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    if (UserSettings)
    {
        TArray<FIntPoint> SupportedResolutions;
        // UserSettings->GetSupportedFullscreenResolutions(SupportedResolutions);

        if (SupportedResolutions.Num() > 0)
        {
            for (const FIntPoint& Res : SupportedResolutions)
            {
                float AspectRatio = static_cast<float>(Res.X) / static_cast<float>(Res.Y);
                if ((FMath::IsNearlyEqual(AspectRatio, 16.0f / 9.0f, 0.1f) ||
                    FMath::IsNearlyEqual(AspectRatio, 16.0f / 10.0f, 0.1f)) &&
                    !Resolutions.Contains(Res))
                {
                    Resolutions.Add(Res);
                }
            }
        }
    }

    // Fallback common resolutions
    if (Resolutions.Num() == 0)
    {
        Resolutions.Add(FIntPoint(1280, 720));   // HD
        Resolutions.Add(FIntPoint(1920, 1080));  // Full HD
        Resolutions.Add(FIntPoint(2560, 1440));  // QHD
        Resolutions.Add(FIntPoint(3840, 2160));  // 4K
    }

    Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
        {
            return (A.X * A.Y) < (B.X * B.Y);
        });

    return Resolutions;
}

FIntPoint UGraphicWidget::IndexToResolution(int32 Index) const
{
    TArray<FIntPoint> Resolutions = GetAvailableResolutions();
    if (Index >= 0 && Index < Resolutions.Num())
    {
        return Resolutions[Index];
    }
    return FIntPoint(1920, 1080); // Default
}

int32 UGraphicWidget::ResolutionToIndex(FIntPoint Resolution) const
{
    TArray<FIntPoint> Resolutions = GetAvailableResolutions();

    for (int32 i = 0; i < Resolutions.Num(); i++)
    {
        if (Resolutions[i] == Resolution)
        {
            return i;
        }
    }

    // Find closest match
    int32 ClosestIndex = 1; // Default to Full HD
    int32 ClosestDiff = INT_MAX;

    for (int32 i = 0; i < Resolutions.Num(); i++)
    {
        int32 Diff = FMath::Abs((Resolutions[i].X * Resolutions[i].Y) - (Resolution.X * Resolution.Y));
        if (Diff < ClosestDiff)
        {
            ClosestDiff = Diff;
            ClosestIndex = i;
        }
    }

    return ClosestIndex;
}

int32 UGraphicWidget::IndexToFrameRateCap(int32 Index) const
{
    TArray<int32> FrameRates = { 30, 60, 90, 120, 144, 0 }; // 0 = Unlimited
    if (Index >= 0 && Index < FrameRates.Num())
    {
        return FrameRates[Index];
    }
    return 60; // Default
}

int32 UGraphicWidget::FrameRateCapToIndex(int32 Value) const
{
    TArray<int32> FrameRates = { 30, 60, 90, 120, 144, 0 };

    for (int32 i = 0; i < FrameRates.Num(); i++)
    {
        if (FrameRates[i] == Value)
        {
            return i;
        }
    }

    return 1; // Default to 60 FPS
}

float UGraphicWidget::IndexToFOV(int32 Index) const
{
    TArray<float> FOVs = { 70.0f, 80.0f, 90.0f, 100.0f, 110.0f, 120.0f };
    if (Index >= 0 && Index < FOVs.Num())
    {
        return FOVs[Index];
    }
    return 90.0f; // Default
}

int32 UGraphicWidget::FOVToIndex(float Value) const
{
    TArray<float> FOVs = { 70.0f, 80.0f, 90.0f, 100.0f, 110.0f, 120.0f };

    int32 ClosestIndex = 2; // Default to 90°
    float ClosestDiff = FLT_MAX;

    for (int32 i = 0; i < FOVs.Num(); i++)
    {
        float Diff = FMath::Abs(Value - FOVs[i]);
        if (Diff < ClosestDiff)
        {
            ClosestDiff = Diff;
            ClosestIndex = i;
        }
    }

    return ClosestIndex;
}

float UGraphicWidget::IndexToPercentage(int32 Index) const
{
    TArray<float> Percentages = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
    if (Index >= 0 && Index < Percentages.Num())
    {
        return Percentages[Index];
    }
    return 0.5f; // Default
}

int32 UGraphicWidget::PercentageToIndex(float Value) const
{
    TArray<float> Percentages = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };

    int32 ClosestIndex = 2; // Default to 50%
    float ClosestDiff = FLT_MAX;

    for (int32 i = 0; i < Percentages.Num(); i++)
    {
        float Diff = FMath::Abs(Value - Percentages[i]);
        if (Diff < ClosestDiff)
        {
            ClosestDiff = Diff;
            ClosestIndex = i;
        }
    }

    return ClosestIndex;
}

void UGraphicWidget::MarkAsCustomPreset()
{
    if (CurrentSettings.QualityPreset != EE_GraphicsQuality::Custom)
    {
        CurrentSettings.QualityPreset = EE_GraphicsQuality::Custom;
        if (QualityPresetRow)
        {
            bIsLoadingSettings = true;
            QualityPresetRow->SetCurrentSelection(static_cast<int32>(EE_GraphicsQuality::Custom), false);
            bIsLoadingSettings = false;
        }
    }
}

// Selection callbacks

void UGraphicWidget::OnQualityPresetChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;

    EE_GraphicsQuality Quality = static_cast<EE_GraphicsQuality>(NewIndex);
    CurrentSettings.QualityPreset = Quality;

    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Quality preset changed to index %d (local)"), NewIndex);

    // Auto-adjust settings based on preset if subsystem is available
    if (Quality != EE_GraphicsQuality::Custom && SettingsSubsystem)
    {
        FS_GraphicsSettings PresetSettings = SettingsSubsystem->GetGraphicsPreset(Quality);

        // Copy preset values (except resolution and FPS cap)
        CurrentSettings.bRayTracingEnabled = PresetSettings.bRayTracingEnabled;
        CurrentSettings.RayTracingQuality = PresetSettings.RayTracingQuality;
        CurrentSettings.ShadowQuality = PresetSettings.ShadowQuality;
        CurrentSettings.TextureQuality = PresetSettings.TextureQuality;
        CurrentSettings.AntiAliasingMethod = PresetSettings.AntiAliasingMethod;
        CurrentSettings.MotionBlurAmount = PresetSettings.MotionBlurAmount;

        // Update UI to reflect auto-adjusted values (without triggering callbacks)
        bIsLoadingSettings = true;

        if (RayTracingRow)
            RayTracingRow->SetCurrentSelection(CurrentSettings.bRayTracingEnabled ? 1 : 0, false);
        if (RayTracingQualityRow)
        {
            RayTracingQualityRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.RayTracingQuality), false);
            RayTracingQualityRow->SetIsEnabled(CurrentSettings.bRayTracingEnabled);
        }
        if (ShadowQualityRow)
            ShadowQualityRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.ShadowQuality), false);
        if (TextureQualityRow)
            TextureQualityRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.TextureQuality), false);
        if (AntiAliasingRow)
            AntiAliasingRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.AntiAliasingMethod), false);
        if (MotionBlurRow)
            MotionBlurRow->SetCurrentSelection(PercentageToIndex(CurrentSettings.MotionBlurAmount), false);

        bIsLoadingSettings = false;

        SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
    }
}

void UGraphicWidget::OnResolutionChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;

    FIntPoint Resolution = IndexToResolution(NewIndex);
    CurrentSettings.ResolutionX = Resolution.X;
    CurrentSettings.ResolutionY = Resolution.Y;

    MarkAsCustomPreset();

    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Resolution changed to %dx%d (local)"), Resolution.X, Resolution.Y);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnVSyncChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bVSyncEnabled = (NewIndex == 1);
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: VSync changed to %s (local)"), CurrentSettings.bVSyncEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnFrameRateCapChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.FrameRateCap = IndexToFrameRateCap(NewIndex);
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: FrameRateCap changed to %d (local)"), CurrentSettings.FrameRateCap);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnRayTracingChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bRayTracingEnabled = (NewIndex == 1);
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: RayTracing changed to %s (local)"), CurrentSettings.bRayTracingEnabled ? TEXT("true") : TEXT("false"));
    if (RayTracingQualityRow) RayTracingQualityRow->SetIsEnabled(CurrentSettings.bRayTracingEnabled);
    if (SettingsSubsystem)  SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnRayTracingQualityChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.RayTracingQuality = static_cast<EE_RayTracingQuality>(NewIndex);
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: RayTracingQuality changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnShadowQualityChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.ShadowQuality = static_cast<EE_ShadowQuality>(NewIndex);
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: ShadowQuality changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnTextureQualityChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.TextureQuality = static_cast<EE_TextureQuality>(NewIndex);
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: TextureQuality changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnAntiAliasingChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.AntiAliasingMethod = static_cast<EE_AntiAliasingMethod>(NewIndex); 
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: AntiAliasingMethod changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnMotionBlurChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.MotionBlurAmount = IndexToPercentage(NewIndex);
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: MotionBlur changed to %.2f (local)"), CurrentSettings.MotionBlurAmount);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}

void UGraphicWidget::OnFieldOfViewChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.FieldOfView = IndexToFOV(NewIndex);
    MarkAsCustomPreset();
    UE_LOG(LogTemp, Log, TEXT("GraphicWidget: FieldOfView changed to %.1f (local)"), CurrentSettings.FieldOfView);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGraphicsSettings(CurrentSettings);
}