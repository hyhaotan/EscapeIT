#pragma once

#include "CoreMinimal.h"
#include "Data/SettingsStructs.h"

class UWorld;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * Handler for accessibility settings application
 * Manages colorblind modes, text settings, motion reduction, and other accessibility features
 */
class FAccessibilitySettingsHandler
{
public:
    /** Initialize the accessibility handler */
    static void Initialize(UWorld* World);

    /** Apply accessibility settings to the engine */
    static void ApplyToEngine(const FS_AccessibilitySettings& Settings, UWorld* World);

    /** Individual setting applications */
    static void SetTextSize(EE_TextSize Size);
    static void SetTextContrast(EE_TextContrast Contrast);
    static void SetDyslexiaFont(bool bEnabled);
    static void SetColorBlindMode(EE_ColorBlindMode Mode, UWorld* World);
    static void SetHighContrastUI(bool bEnabled, UWorld* World);
    static void SetReducedMotion(bool bEnabled);
    static void SetPhotosensitivityMode(EE_PhotosensitivityMode Mode, UWorld* World);
    static void SetScreenReader(bool bEnabled);
    static void SetSoundCuesVisualization(bool bEnabled);
    static void SetHapticFeedback(bool bEnabled);
    static void SetSingleHandedMode(EE_SingleHandedMode Mode);
    static void SetHoldToActivate(bool bEnabled, float HoldTime);

    /** Get current settings */
    static float GetTextSizeScale() { return CachedTextSizeScale; }
    static float GetTextContrastScale() { return CachedTextContrastScale; }
    static bool IsReducedMotionEnabled() { return bCachedReducedMotion; }
    static bool IsHighContrastEnabled() { return bCachedHighContrast; }

    /** Check if handler is initialized */
    static bool IsInitialized() { return bIsInitialized; }

    /** Cleanup resources */
    static void Shutdown();

private:
    /** Apply color blind filter to post process */
    static void ApplyColorBlindFilter(EE_ColorBlindMode Mode, UWorld* World);

    /** Apply photosensitivity reduction */
    static void ApplyPhotosensitivitySettings(EE_PhotosensitivityMode Mode, UWorld* World);

    /** Update UI scaling */
    static void UpdateUIScaling(float Scale);

    /** Apply high contrast materials */
    static void ApplyHighContrastMaterials(bool bEnabled, UWorld* World);

    /** Get text size multiplier */
    static float GetTextSizeMultiplier(EE_TextSize Size);

    /** Get text contrast multiplier */
    static float GetTextContrastMultiplier(EE_TextContrast Contrast);

    /** Cached values */
    static float CachedTextSizeScale;
    static float CachedTextContrastScale;
    static bool bCachedReducedMotion;
    static bool bCachedHighContrast;
    static bool bCachedDyslexiaFont;
    static bool bCachedScreenReader;
    static bool bCachedSoundVisualization;
    static bool bCachedHapticFeedback;
    static EE_ColorBlindMode CachedColorBlindMode;
    static EE_PhotosensitivityMode CachedPhotosensitivityMode;
    static EE_SingleHandedMode CachedSingleHandedMode;
    static bool bCachedHoldToActivate;
    static float CachedHoldActivationTime;

    /** Post process material for color blind filters */
    static UMaterialInstanceDynamic* ColorBlindPostProcessMaterial;

    /** Initialization flag */
    static bool bIsInitialized;
};