
#include "Settings/Validators/SettingsValidator.h"
#include "Data/SettingsConstants.h"

bool FSettingsValidator::ValidateAllSettings(const FS_AllSettings& Settings, FString& OutErrorMessage)
{
    TArray<FString> Errors;
    FString ErrorMsg;

    if (!ValidateGraphicsSettings(Settings.GraphicsSettings, ErrorMsg))
    {
        Errors.Add(FString::Printf(TEXT("Graphics: %s"), *ErrorMsg));
    }

    if (!ValidateAudioSettings(Settings.AudioSettings, ErrorMsg))
    {
        Errors.Add(FString::Printf(TEXT("Audio: %s"), *ErrorMsg));
    }

    if (!ValidateGameplaySettings(Settings.GameplaySettings, ErrorMsg))
    {
        Errors.Add(FString::Printf(TEXT("Gameplay: %s"), *ErrorMsg));
    }

    if (!ValidateControlSettings(Settings.ControlSettings, ErrorMsg))
    {
        Errors.Add(FString::Printf(TEXT("Controls: %s"), *ErrorMsg));
    }

    if (!ValidateAccessibilitySettings(Settings.AccessibilitySettings, ErrorMsg))
    {
        Errors.Add(FString::Printf(TEXT("Accessibility: %s"), *ErrorMsg));
    }

    if (Errors.Num() > 0)
    {
        OutErrorMessage = FString::Join(Errors, TEXT("; "));
        return false;
    }

    return true;
}

bool FSettingsValidator::ValidateGraphicsSettings(const FS_GraphicsSettings& Settings, FString& OutError)
{
    using namespace SettingsConstants;

    if (!ValidateRange(Settings.ResolutionX, MIN_RESOLUTION_X, MAX_RESOLUTION_X, TEXT("Resolution X"), OutError))
        return false;

    if (!ValidateRange(Settings.ResolutionY, MIN_RESOLUTION_Y, MAX_RESOLUTION_Y, TEXT("Resolution Y"), OutError))
        return false;

    if (!ValidateRange(Settings.FieldOfView, MIN_FOV, MAX_FOV, TEXT("Field of View"), OutError))
        return false;

    if (!ValidateRange(Settings.MotionBlurAmount, MIN_VOLUME, MAX_VOLUME, TEXT("Motion Blur Amount"), OutError))
        return false;

    if (Settings.FrameRateCap != UNLIMITED_FRAME_RATE)
    {
        if (!ValidateRange(Settings.FrameRateCap, MIN_FRAME_RATE, MAX_FRAME_RATE, TEXT("Frame Rate Cap"), OutError))
            return false;
    }

    return true;
}

bool FSettingsValidator::ValidateAudioSettings(const FS_AudioSettings& Settings, FString& OutError)
{
    using namespace SettingsConstants;

    if (!ValidateRange(Settings.MasterVolume, MIN_VOLUME, MAX_VOLUME, TEXT("Master Volume"), OutError))
        return false;

    if (!ValidateRange(Settings.MusicVolume, MIN_VOLUME, MAX_VOLUME, TEXT("Music Volume"), OutError))
        return false;

    if (!ValidateRange(Settings.SFXVolume, MIN_VOLUME, MAX_VOLUME, TEXT("SFX Volume"), OutError))
        return false;

    if (!ValidateRange(Settings.AmbientVolume, MIN_VOLUME, MAX_VOLUME, TEXT("Ambient Volume"), OutError))
        return false;

    if (!ValidateRange(Settings.DialogueVolume, MIN_VOLUME, MAX_VOLUME, TEXT("Dialogue Volume"), OutError))
        return false;

    if (!ValidateRange(Settings.UIVolume, MIN_VOLUME, MAX_VOLUME, TEXT("UI Volume"), OutError))
        return false;

    return true;
}

bool FSettingsValidator::ValidateGameplaySettings(const FS_GameplaySettings& Settings, FString& OutError)
{
    using namespace SettingsConstants;

    if (!ValidateRange(Settings.SanityDrainMultiplier, 0.5f, 2.0f, TEXT("Sanity Drain Multiplier"), OutError))
        return false;

    if (!ValidateRange(Settings.EntityDetectionRangeMultiplier, 0.5f, 2.0f, TEXT("Entity Detection Range Multiplier"), OutError))
        return false;

    if (!ValidateRange(Settings.AutoRevealHintTime, MIN_HINT_TIME, MAX_HINT_TIME, TEXT("Auto Reveal Hint Time"), OutError))
        return false;

    if (!ValidateRange(Settings.CameraShakeMagnitude, 0.0f, 2.0f, TEXT("Camera Shake Magnitude"), OutError))
        return false;

    if (!ValidateRange(Settings.ScreenBlurAmount, MIN_VOLUME, MAX_VOLUME, TEXT("Screen Blur Amount"), OutError))
        return false;

    return true;
}

bool FSettingsValidator::ValidateControlSettings(const FS_ControlSettings& Settings, FString& OutError)
{
    using namespace SettingsConstants;

    if (!ValidateRange(Settings.MouseSensitivity, MIN_SENSITIVITY, MAX_SENSITIVITY, TEXT("Mouse Sensitivity"), OutError))
        return false;

    if (!ValidateRange(Settings.CameraZoomSensitivity, MIN_VOLUME, MAX_VOLUME, TEXT("Camera Zoom Sensitivity"), OutError))
        return false;

    if (!ValidateRange(Settings.GamepadSensitivity, MIN_SENSITIVITY, MAX_SENSITIVITY, TEXT("Gamepad Sensitivity"), OutError))
        return false;

    if (!ValidateRange(Settings.GamepadDeadzone, MIN_DEADZONE, MAX_DEADZONE, TEXT("Gamepad Deadzone"), OutError))
        return false;

    if (!ValidateRange(Settings.GamepadVibrationIntensity, MIN_VOLUME, MAX_VOLUME, TEXT("Gamepad Vibration Intensity"), OutError))
        return false;

    return true;
}

bool FSettingsValidator::ValidateAccessibilitySettings(const FS_AccessibilitySettings& Settings, FString& OutError)
{
    using namespace SettingsConstants;

    if (!ValidateRange(Settings.HoldActivationTime, MIN_HOLD_TIME, MAX_HOLD_TIME, TEXT("Hold Activation Time"), OutError))
        return false;

    return true;
}

bool FSettingsValidator::ValidateRange(float Value, float Min, float Max, const FString& FieldName, FString& OutError)
{
    if (Value < Min || Value > Max)
    {
        OutError = FString::Printf(TEXT("Invalid %s: %.2f (range: %.2f - %.2f)"), *FieldName, Value, Min, Max);
        return false;
    }
    return true;
}

bool FSettingsValidator::ValidateRange(int32 Value, int32 Min, int32 Max, const FString& FieldName, FString& OutError)
{
    if (Value < Min || Value > Max)
    {
        OutError = FString::Printf(TEXT("Invalid %s: %d (range: %d - %d)"), *FieldName, Value, Min, Max);
        return false;
    }
    return true;
}