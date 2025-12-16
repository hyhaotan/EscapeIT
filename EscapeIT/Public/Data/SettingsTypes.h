#pragma once

#include "CoreMinimal.h"
#include "SettingsTypes.generated.h"

// ===== ENUMS =====

UENUM(BlueprintType)
enum class EE_GraphicsQuality : uint8
{
    Low = 0 UMETA(DisplayName = "Low"),
    Medium = 1 UMETA(DisplayName = "Medium"),
    High = 2 UMETA(DisplayName = "High"),
    Ultra = 3 UMETA(DisplayName = "Ultra"),
    Custom = 4 UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class ESettingsCategory : uint8
{
    All = 0 UMETA(DisplayName = "All"),
    Gameplay = 1 UMETA(DisplayName = "Gameplay"),
    Graphics = 2 UMETA(DisplayName = "Graphics"),
    Audio = 3 UMETA(DisplayName = "Audio"),
    Controls = 4 UMETA(DisplayName = "Controls"),
    Accessibility = 5 UMETA(DisplayName = "Accessibility")
};

UENUM(BlueprintType)
enum class EE_RayTracingQuality : uint8
{
    Low = 0 UMETA(DisplayName = "Low"),
    Medium = 1 UMETA(DisplayName = "Medium"),
    High = 2 UMETA(DisplayName = "High"),
    Epic = 3 UMETA(DisplayName = "Epic")
};

UENUM(BlueprintType)
enum class EE_ShowFPS : uint8
{
    On = 0 UMETA(DisplayName = "On"),
    Off = 1 UMETA(DisplayName = "Off")
};

UENUM(BlueprintType)
enum class EE_ShadowQuality : uint8
{
    Low = 0 UMETA(DisplayName = "Low"),
    Medium = 1 UMETA(DisplayName = "Medium"),
    High = 2 UMETA(DisplayName = "High"),
    Epic = 3 UMETA(DisplayName = "Epic")
};

UENUM(BlueprintType)
enum class EE_TextureQuality : uint8
{
    Low = 0 UMETA(DisplayName = "Low"),
    Medium = 1 UMETA(DisplayName = "Medium"),
    High = 2 UMETA(DisplayName = "High"),
    Epic = 3 UMETA(DisplayName = "Epic")
};

UENUM(BlueprintType)
enum class EE_AntiAliasingMethod : uint8
{
    None = 0 UMETA(DisplayName = "None"),
    FXAA = 1 UMETA(DisplayName = "FXAA"),
    TAA = 2 UMETA(DisplayName = "TAA"),
    MSAA_2x = 3 UMETA(DisplayName = "MSAA 2x"),
    MSAA_4x = 4 UMETA(DisplayName = "MSAA 4x")
};

UENUM(BlueprintType)
enum class EE_AudioLanguage : uint8
{
    English = 0 UMETA(DisplayName = "English"),
    Vietnamese = 1 UMETA(DisplayName = "Vietnamese"),
    French = 2 UMETA(DisplayName = "French"),
    German = 3 UMETA(DisplayName = "German"),
    Spanish = 4 UMETA(DisplayName = "Spanish"),
    Japanese = 5 UMETA(DisplayName = "Japanese")
};

UENUM(BlueprintType)
enum class EE_AudioOutput : uint8
{
    Stereo = 0 UMETA(DisplayName = "Stereo"),
    Surround_5_1 = 1 UMETA(DisplayName = "Surround 5.1"),
    Surround_7_1 = 2 UMETA(DisplayName = "Surround 7.1"),
    Headphones = 3 UMETA(DisplayName = "Headphones")
};

UENUM(BlueprintType)
enum class EE_DifficultyLevel : uint8
{
    Easy = 0 UMETA(DisplayName = "Easy"),
    Normal = 1 UMETA(DisplayName = "Normal"),
    Hard = 2 UMETA(DisplayName = "Hard"),
    Nightmare = 3 UMETA(DisplayName = "Nightmare")
};

UENUM(BlueprintType)
enum class EE_TextSize : uint8
{
    Small = 0 UMETA(DisplayName = "Small"),
    Normal = 1 UMETA(DisplayName = "Normal"),
    Large = 2 UMETA(DisplayName = "Large"),
    ExtraLarge = 3 UMETA(DisplayName = "Extra Large")
};

UENUM(BlueprintType)
enum class EE_TextContrast : uint8
{
    Normal = 0 UMETA(DisplayName = "Normal"),
    Medium = 1 UMETA(DisplayName = "Medium"),
    High = 2 UMETA(DisplayName = "High")
};

UENUM(BlueprintType)
enum class EE_ColorBlindMode : uint8
{
    None = 0 UMETA(DisplayName = "None"),
    Protanopia = 1 UMETA(DisplayName = "Protanopia"),
    Deuteranopia = 2 UMETA(DisplayName = "Deuteranopia"),
    Tritanopia = 3 UMETA(DisplayName = "Tritanopia")
};

UENUM(BlueprintType)
enum class EE_PhotosensitivityMode : uint8
{
    None = 0 UMETA(DisplayName = "None"),
    Reduced = 1 UMETA(DisplayName = "Reduced"),
    Maximum = 2 UMETA(DisplayName = "Maximum")
};

UENUM(BlueprintType)
enum class EE_SingleHandedMode : uint8
{
    None = 0 UMETA(DisplayName = "None"),
    LeftHand = 1 UMETA(DisplayName = "Left Hand"),
    RightHand = 2 UMETA(DisplayName = "Right Hand")
};

UENUM(BlueprintType)
enum class EE_AudioSpatialization : uint8
{
    None = 0 UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EE_AudioQuality : uint8
{
    Low = 0 UMETA(DisplayName = "Low"),
    Medium = 1 UMETA(DisplayName = "Medium"),
    High = 2 UMETA(DisplayName = "High")
};

// ===== STRUCTS =====

USTRUCT(BlueprintType)
struct FS_GraphicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    EE_GraphicsQuality QualityPreset = EE_GraphicsQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    int32 ResolutionX = 1920;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    int32 ResolutionY = 1080;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    bool bVSyncEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    int32 FrameRateCap = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    bool bRayTracingEnabled = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    bool bFPSEnable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    EE_RayTracingQuality RayTracingQuality = EE_RayTracingQuality::Low;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    EE_ShowFPS FPS = EE_ShowFPS::Off;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    EE_ShadowQuality ShadowQuality = EE_ShadowQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    EE_TextureQuality TextureQuality = EE_TextureQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    EE_AntiAliasingMethod AntiAliasingMethod = EE_AntiAliasingMethod::FXAA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    float MotionBlurAmount = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
    float FieldOfView = 90.0f;

    bool operator==(const FS_GraphicsSettings& Other) const
    {
        return QualityPreset == Other.QualityPreset &&
            ResolutionX == Other.ResolutionX &&
            ResolutionY == Other.ResolutionY &&
            bVSyncEnabled == Other.bVSyncEnabled &&
            FrameRateCap == Other.FrameRateCap &&
            bRayTracingEnabled == Other.bRayTracingEnabled &&
            RayTracingQuality == Other.RayTracingQuality &&
            FPS == Other.FPS &&
            ShadowQuality == Other.ShadowQuality &&
            TextureQuality == Other.TextureQuality &&
            AntiAliasingMethod == Other.AntiAliasingMethod &&
            bFPSEnable == Other.bFPSEnable &&
            FMath::IsNearlyEqual(MotionBlurAmount, Other.MotionBlurAmount) &&
            FMath::IsNearlyEqual(FieldOfView, Other.FieldOfView);
    }

    bool operator!=(const FS_GraphicsSettings& Other) const
    {
        return !(*this == Other);
    }
};

USTRUCT(BlueprintType)
struct FS_AudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;

    // removed incorrect float AudioLanguage (was a typo). Use CurrentLanguage enum below.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SFXVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DialogueVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float UIVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EE_AudioLanguage CurrentLanguage = EE_AudioLanguage::English;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EE_AudioOutput AudioOutput = EE_AudioOutput::Stereo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bSubtitlesEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bClosedCaptionsEnabled = false;

    bool operator==(const FS_AudioSettings& Other) const
    {
        return FMath::IsNearlyEqual(MasterVolume, Other.MasterVolume) &&
            FMath::IsNearlyEqual(MusicVolume, Other.MusicVolume) &&
            FMath::IsNearlyEqual(SFXVolume, Other.SFXVolume) &&
            FMath::IsNearlyEqual(AmbientVolume, Other.AmbientVolume) &&
            FMath::IsNearlyEqual(DialogueVolume, Other.DialogueVolume) &&
            FMath::IsNearlyEqual(UIVolume, Other.UIVolume) &&
            CurrentLanguage == Other.CurrentLanguage &&
            AudioOutput == Other.AudioOutput &&
            bSubtitlesEnabled == Other.bSubtitlesEnabled &&
            bClosedCaptionsEnabled == Other.bClosedCaptionsEnabled;
    }

    bool operator!=(const FS_AudioSettings& Other) const
    {
        return !(*this == Other);
    }
};

USTRUCT(BlueprintType)
struct FS_GameplaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    EE_DifficultyLevel DifficultyLevel = EE_DifficultyLevel::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float SanityDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float MouseSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float EntityDetectionRangeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bPuzzleHintSystemEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float AutoRevealHintTime = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAllowSkipPuzzles = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bShowObjectiveMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bShowInteractionIndicators = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAutoPickupItems = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float CameraShakeMagnitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float ScreenBlurAmount = 0.5f;

    bool operator==(const FS_GameplaySettings& Other) const
    {
        return DifficultyLevel == Other.DifficultyLevel &&
            FMath::IsNearlyEqual(SanityDrainMultiplier, Other.SanityDrainMultiplier) &&
            FMath::IsNearlyEqual(MouseSensitivity, Other.MouseSensitivity) && // added
            FMath::IsNearlyEqual(EntityDetectionRangeMultiplier, Other.EntityDetectionRangeMultiplier) &&
            bPuzzleHintSystemEnabled == Other.bPuzzleHintSystemEnabled &&
            FMath::IsNearlyEqual(AutoRevealHintTime, Other.AutoRevealHintTime) &&
            bAllowSkipPuzzles == Other.bAllowSkipPuzzles &&
            bShowObjectiveMarkers == Other.bShowObjectiveMarkers &&
            bShowInteractionIndicators == Other.bShowInteractionIndicators &&
            bAutoPickupItems == Other.bAutoPickupItems &&
            FMath::IsNearlyEqual(CameraShakeMagnitude, Other.CameraShakeMagnitude) &&
            FMath::IsNearlyEqual(ScreenBlurAmount, Other.ScreenBlurAmount);
    }

    bool operator!=(const FS_GameplaySettings& Other) const
    {
        return !(*this == Other);
    }
};

USTRUCT(BlueprintType)
struct FS_ControlSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float MouseSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bInvertMouseY = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float CameraZoomSensitivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float GamepadSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float GamepadDeadzone = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bInvertGamepadY = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bAutoSprintEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bCrouchToggle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bFlashlightToggle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bGamepadVibrationEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float GamepadVibrationIntensity = 1.0f;

    bool operator==(const FS_ControlSettings& Other) const
    {
        return FMath::IsNearlyEqual(MouseSensitivity, Other.MouseSensitivity) &&
            bInvertMouseY == Other.bInvertMouseY &&
            FMath::IsNearlyEqual(CameraZoomSensitivity, Other.CameraZoomSensitivity) &&
            FMath::IsNearlyEqual(GamepadSensitivity, Other.GamepadSensitivity) &&
            FMath::IsNearlyEqual(GamepadDeadzone, Other.GamepadDeadzone) &&
            bInvertGamepadY == Other.bInvertGamepadY &&
            bAutoSprintEnabled == Other.bAutoSprintEnabled &&
            bCrouchToggle == Other.bCrouchToggle &&
            bFlashlightToggle == Other.bFlashlightToggle &&
            bGamepadVibrationEnabled == Other.bGamepadVibrationEnabled &&
            FMath::IsNearlyEqual(GamepadVibrationIntensity, Other.GamepadVibrationIntensity);
    }

    bool operator!=(const FS_ControlSettings& Other) const
    {
        return !(*this == Other);
    }
};

USTRUCT(BlueprintType)
struct FS_AccessibilitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    EE_TextSize TextSize = EE_TextSize::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    EE_TextContrast TextContrast = EE_TextContrast::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bDyslexiaFontEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    EE_ColorBlindMode ColorBlindMode = EE_ColorBlindMode::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bHighContrastUIEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bReducedMotionEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    EE_PhotosensitivityMode PhotosensitivityMode = EE_PhotosensitivityMode::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bScreenReaderEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bSoundCuesVisualizationEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bHapticFeedbackEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    EE_SingleHandedMode SingleHandedMode = EE_SingleHandedMode::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bEnableHoldToActivate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    float HoldActivationTime = 0.5f;

    bool operator==(const FS_AccessibilitySettings& Other) const
    {
        return TextSize == Other.TextSize &&
            TextContrast == Other.TextContrast &&
            bDyslexiaFontEnabled == Other.bDyslexiaFontEnabled &&
            ColorBlindMode == Other.ColorBlindMode &&
            bHighContrastUIEnabled == Other.bHighContrastUIEnabled &&
            bReducedMotionEnabled == Other.bReducedMotionEnabled &&
            PhotosensitivityMode == Other.PhotosensitivityMode &&
            bScreenReaderEnabled == Other.bScreenReaderEnabled &&
            bSoundCuesVisualizationEnabled == Other.bSoundCuesVisualizationEnabled &&
            bHapticFeedbackEnabled == Other.bHapticFeedbackEnabled &&
            SingleHandedMode == Other.SingleHandedMode &&
            bEnableHoldToActivate == Other.bEnableHoldToActivate &&
            FMath::IsNearlyEqual(HoldActivationTime, Other.HoldActivationTime);
    }

    bool operator!=(const FS_AccessibilitySettings& Other) const
    {
        return !(*this == Other);
    }
};

USTRUCT(BlueprintType)
struct FS_AllSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "All")
    FS_GraphicsSettings GraphicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "All")
    FS_AudioSettings AudioSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "All")
    FS_GameplaySettings GameplaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "All")
    FS_ControlSettings ControlSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "All")
    FS_AccessibilitySettings AccessibilitySettings;
};

USTRUCT(BlueprintType)
struct FS_DifficultyMultiplier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float SanityDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float AIDetectionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float AISpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float EntityChaseMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float BatteryLifeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float PuzzleHintAvailability = 1.0f;
};
