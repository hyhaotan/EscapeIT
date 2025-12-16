#include "Settings/Handlers/AccessibilitySettingsHandler.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PostProcessComponent.h"

// Static member initialization
float FAccessibilitySettingsHandler::CachedTextSizeScale = 1.0f;
float FAccessibilitySettingsHandler::CachedTextContrastScale = 1.0f;
bool FAccessibilitySettingsHandler::bCachedReducedMotion = false;
bool FAccessibilitySettingsHandler::bCachedHighContrast = false;
bool FAccessibilitySettingsHandler::bCachedDyslexiaFont = false;
bool FAccessibilitySettingsHandler::bCachedScreenReader = false;
bool FAccessibilitySettingsHandler::bCachedSoundVisualization = false;
bool FAccessibilitySettingsHandler::bCachedHapticFeedback = true;
EE_ColorBlindMode FAccessibilitySettingsHandler::CachedColorBlindMode = EE_ColorBlindMode::None;
EE_PhotosensitivityMode FAccessibilitySettingsHandler::CachedPhotosensitivityMode = EE_PhotosensitivityMode::None;
EE_SingleHandedMode FAccessibilitySettingsHandler::CachedSingleHandedMode = EE_SingleHandedMode::None;
bool FAccessibilitySettingsHandler::bCachedHoldToActivate = false;
float FAccessibilitySettingsHandler::CachedHoldActivationTime = 0.5f;
UMaterialInstanceDynamic* FAccessibilitySettingsHandler::ColorBlindPostProcessMaterial = nullptr;
bool FAccessibilitySettingsHandler::bIsInitialized = false;

void FAccessibilitySettingsHandler::Initialize(UWorld* World)
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityHandler: Already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Initializing..."));

    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AccessibilityHandler: World is null"));
        return;
    }

    // Initialize default values
    CachedTextSizeScale = 1.0f;
    CachedTextContrastScale = 1.0f;
    bCachedReducedMotion = false;
    bCachedHighContrast = false;

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Initialized successfully"));
}

void FAccessibilitySettingsHandler::Shutdown()
{
    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Shutting down"));

    // Clear post process materials
    ColorBlindPostProcessMaterial = nullptr;

    // Reset cached values
    CachedTextSizeScale = 1.0f;
    CachedTextContrastScale = 1.0f;
    bCachedReducedMotion = false;
    bCachedHighContrast = false;

    bIsInitialized = false;
}

void FAccessibilitySettingsHandler::ApplyToEngine(const FS_AccessibilitySettings& Settings, UWorld* World)
{
    if (!bIsInitialized)
    {
        Initialize(World);
    }

    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("AccessibilityHandler: Cannot apply settings - initialization failed"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Applying accessibility settings"));

    // Apply all settings
    SetTextSize(Settings.TextSize);
    SetTextContrast(Settings.TextContrast);
    SetDyslexiaFont(Settings.bDyslexiaFontEnabled);
    SetColorBlindMode(Settings.ColorBlindMode, World);
    SetHighContrastUI(Settings.bHighContrastUIEnabled, World);
    SetReducedMotion(Settings.bReducedMotionEnabled);
    SetPhotosensitivityMode(Settings.PhotosensitivityMode, World);
    SetScreenReader(Settings.bScreenReaderEnabled);
    SetSoundCuesVisualization(Settings.bSoundCuesVisualizationEnabled);
    SetHapticFeedback(Settings.bHapticFeedbackEnabled);
    SetSingleHandedMode(Settings.SingleHandedMode);
    SetHoldToActivate(Settings.bEnableHoldToActivate, Settings.HoldActivationTime);

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: All accessibility settings applied"));
}

// ===== INDIVIDUAL SETTERS =====

void FAccessibilitySettingsHandler::SetTextSize(EE_TextSize Size)
{
    CachedTextSizeScale = GetTextSizeMultiplier(Size);

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Text size set to %d (scale: %.2f)"),
        static_cast<int32>(Size), CachedTextSizeScale);

    // Note: Actual UI text scaling should be handled by your UI system
    // You can broadcast an event here for UI to update
}

void FAccessibilitySettingsHandler::SetTextContrast(EE_TextContrast Contrast)
{
    CachedTextContrastScale = GetTextContrastMultiplier(Contrast);

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Text contrast set to %d (scale: %.2f)"),
        static_cast<int32>(Contrast), CachedTextContrastScale);

    // Note: Actual contrast changes should be handled by your UI system
}

void FAccessibilitySettingsHandler::SetDyslexiaFont(bool bEnabled)
{
    bCachedDyslexiaFont = bEnabled;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Dyslexia font %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));

    // Note: Font switching should be handled by your UI system
    // Common dyslexia-friendly fonts: OpenDyslexic, Comic Sans, Arial
}

void FAccessibilitySettingsHandler::SetColorBlindMode(EE_ColorBlindMode Mode, UWorld* World)
{
    CachedColorBlindMode = Mode;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Color blind mode set to %d"),
        static_cast<int32>(Mode));

    if (World)
    {
        ApplyColorBlindFilter(Mode, World);
    }
}

void FAccessibilitySettingsHandler::SetHighContrastUI(bool bEnabled, UWorld* World)
{
    bCachedHighContrast = bEnabled;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: High contrast UI %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));

    if (World)
    {
        ApplyHighContrastMaterials(bEnabled, World);
    }
}

void FAccessibilitySettingsHandler::SetReducedMotion(bool bEnabled)
{
    bCachedReducedMotion = bEnabled;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Reduced motion %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));

    // When enabled, should reduce:
    // - Camera shake
    // - Motion blur
    // - Screen shake effects
    // - Rapid animations

    if (bEnabled)
    {
        // Disable motion blur through console variable
        if (IConsoleVariable* MotionBlurCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality")))
        {
            MotionBlurCVar->Set(0);
        }

        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Motion effects reduced"));
    }
}

void FAccessibilitySettingsHandler::SetPhotosensitivityMode(EE_PhotosensitivityMode Mode, UWorld* World)
{
    CachedPhotosensitivityMode = Mode;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Photosensitivity mode set to %d"),
        static_cast<int32>(Mode));

    if (World)
    {
        ApplyPhotosensitivitySettings(Mode, World);
    }
}

void FAccessibilitySettingsHandler::SetScreenReader(bool bEnabled)
{
    bCachedScreenReader = bEnabled;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Screen reader %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));

    // Note: Actual screen reader integration requires platform-specific APIs
    // Windows: SAPI, macOS: VoiceOver API, etc.
}

void FAccessibilitySettingsHandler::SetSoundCuesVisualization(bool bEnabled)
{
    bCachedSoundVisualization = bEnabled;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Sound cues visualization %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));

    // When enabled, should show visual indicators for:
    // - Footsteps
    // - Enemy sounds
    // - Environmental audio cues
    // - Dialogue direction
}

void FAccessibilitySettingsHandler::SetHapticFeedback(bool bEnabled)
{
    bCachedHapticFeedback = bEnabled;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Haptic feedback %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));

    // This affects controller vibration and force feedback
}

void FAccessibilitySettingsHandler::SetSingleHandedMode(EE_SingleHandedMode Mode)
{
    CachedSingleHandedMode = Mode;

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Single-handed mode set to %d"),
        static_cast<int32>(Mode));

    // When enabled, should remap controls for one-handed play
    // This requires input remapping implementation
}

void FAccessibilitySettingsHandler::SetHoldToActivate(bool bEnabled, float HoldTime)
{
    bCachedHoldToActivate = bEnabled;
    CachedHoldActivationTime = FMath::Clamp(HoldTime, 0.1f, 5.0f);

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Hold to activate %s (time: %.2fs)"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"), CachedHoldActivationTime);

    // When enabled, interactions require holding button instead of pressing
}

// ===== PRIVATE HELPER FUNCTIONS =====

void FAccessibilitySettingsHandler::ApplyColorBlindFilter(EE_ColorBlindMode Mode, UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityHandler: Cannot apply color blind filter - World is null"));
        return;
    }

    // Find or create post process volume
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);

    APostProcessVolume* PPVolume = nullptr;

    if (FoundActors.Num() > 0)
    {
        PPVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }

    if (!PPVolume)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityHandler: No post process volume found"));
        return;
    }

    // Apply color blind settings through post process
    FPostProcessSettings& PPSettings = PPVolume->Settings;

    switch (Mode)
    {
    case EE_ColorBlindMode::None:
        // Disable color grading
        PPSettings.bOverride_ColorSaturation = false;
        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Color blind filter disabled"));
        break;

    case EE_ColorBlindMode::Protanopia:
        // Red-blind (difficulty seeing red)
        PPSettings.bOverride_ColorSaturation = true;
        PPSettings.ColorSaturation = FVector4(0.567f, 0.433f, 0.0f, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Protanopia filter applied"));
        break;

    case EE_ColorBlindMode::Deuteranopia:
        // Green-blind (difficulty seeing green)
        PPSettings.bOverride_ColorSaturation = true;
        PPSettings.ColorSaturation = FVector4(0.625f, 0.375f, 0.0f, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Deuteranopia filter applied"));
        break;

    case EE_ColorBlindMode::Tritanopia:
        // Blue-blind (difficulty seeing blue)
        PPSettings.bOverride_ColorSaturation = true;
        PPSettings.ColorSaturation = FVector4(0.95f, 0.05f, 0.0f, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Tritanopia filter applied"));
        break;
    }
}

void FAccessibilitySettingsHandler::ApplyPhotosensitivitySettings(EE_PhotosensitivityMode Mode, UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityHandler: Cannot apply photosensitivity settings - World is null"));
        return;
    }

    // Find post process volume
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);

    APostProcessVolume* PPVolume = nullptr;

    if (FoundActors.Num() > 0)
    {
        PPVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }

    if (!PPVolume)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityHandler: No post process volume found"));
        return;
    }

    FPostProcessSettings& PPSettings = PPVolume->Settings;

    switch (Mode)
    {
    case EE_PhotosensitivityMode::None:
        // Normal settings
        PPSettings.bOverride_BloomIntensity = false;
        PPSettings.bOverride_LensFlareIntensity = false;
        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Photosensitivity mode disabled"));
        break;

    case EE_PhotosensitivityMode::Reduced:
        // Reduce bright flashes
        PPSettings.bOverride_BloomIntensity = true;
        PPSettings.BloomIntensity = 0.5f;
        PPSettings.bOverride_LensFlareIntensity = true;
        PPSettings.LensFlareIntensity = 0.5f;
        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Reduced photosensitivity mode applied"));
        break;

    case EE_PhotosensitivityMode::Maximum:
        // Minimize all bright effects
        PPSettings.bOverride_BloomIntensity = true;
        PPSettings.BloomIntensity = 0.1f;
        PPSettings.bOverride_LensFlareIntensity = true;
        PPSettings.LensFlareIntensity = 0.0f;
        PPSettings.bOverride_AutoExposureMinBrightness = true;
        PPSettings.AutoExposureMinBrightness = 0.5f;
        PPSettings.bOverride_AutoExposureMaxBrightness = true;
        PPSettings.AutoExposureMaxBrightness = 2.0f;
        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Maximum photosensitivity protection applied"));
        break;
    }

    // Also disable screen flashes in reduced motion
    if (bCachedReducedMotion)
    {
        PPSettings.bOverride_MotionBlurAmount = true;
        PPSettings.MotionBlurAmount = 0.0f;
    }
}

void FAccessibilitySettingsHandler::UpdateUIScaling(float Scale)
{
    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Updating UI scaling to %.2f"), Scale);

    // Note: Actual UI scaling should be implemented in your widget base class
    // or through UMG DPI scaling
}

void FAccessibilitySettingsHandler::ApplyHighContrastMaterials(bool bEnabled, UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityHandler: Cannot apply high contrast - World is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: High contrast materials %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));

    // Find post process volume
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);

    APostProcessVolume* PPVolume = nullptr;

    if (FoundActors.Num() > 0)
    {
        PPVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }

    if (!PPVolume)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityHandler: No post process volume found"));
        return;
    }

    FPostProcessSettings& PPSettings = PPVolume->Settings;

    if (bEnabled)
    {
        // Increase contrast
        PPSettings.bOverride_ColorContrast = true;
        PPSettings.ColorContrast = FVector4(1.3f, 1.3f, 1.3f, 1.0f);

        // Increase saturation
        PPSettings.bOverride_ColorSaturation = true;
        PPSettings.ColorSaturation = FVector4(1.2f, 1.2f, 1.2f, 1.0f);

        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: High contrast applied"));
    }
    else
    {
        // Reset to normal
        PPSettings.bOverride_ColorContrast = false;
        PPSettings.bOverride_ColorSaturation = false;

        UE_LOG(LogTemp, Log, TEXT("AccessibilityHandler: Normal contrast restored"));
    }
}

float FAccessibilitySettingsHandler::GetTextSizeMultiplier(EE_TextSize Size)
{
    switch (Size)
    {
    case EE_TextSize::Small:
        return 0.85f;
    case EE_TextSize::Normal:
        return 1.0f;
    case EE_TextSize::Large:
        return 1.25f;
    case EE_TextSize::ExtraLarge:
        return 1.5f;
    default:
        return 1.0f;
    }
}

float FAccessibilitySettingsHandler::GetTextContrastMultiplier(EE_TextContrast Contrast)
{
    switch (Contrast)
    {
    case EE_TextContrast::Normal:
        return 1.0f;
    case EE_TextContrast::Medium:
        return 1.2f;
    case EE_TextContrast::High:
        return 1.5f;
    default:
        return 1.0f;
    }
}