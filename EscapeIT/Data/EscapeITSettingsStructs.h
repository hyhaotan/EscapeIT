#pragma once

#include "CoreMinimal.h"
#include "EscapeITEnums.h"
#include "EscapeITSettingsStructs.generated.h"

/**
 * Graphics Settings Structure
 */
USTRUCT(BlueprintType)
struct FS_GraphicsSettings
{
	GENERATED_BODY()

	// Resolution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Resolution")
	int32 ResolutionX = 1920;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Resolution")
	int32 ResolutionY = 1080;

	// Quality & Performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Quality")
	EE_GraphicsQuality QualityPreset = EE_GraphicsQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Performance")
	int32 FrameRateCap = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Performance")
	bool bVSyncEnabled = true;

	// Advanced Graphics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Advanced")
	bool bRayTracingEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Advanced")
	EE_RayTracingQuality RayTracingQuality = EE_RayTracingQuality::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Advanced")
	EE_ShadowQuality ShadowQuality = EE_ShadowQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Advanced")
	EE_TextureQuality TextureQuality = EE_TextureQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|Advanced")
	EE_AntiAliasingMethod AntiAliasingMethod = EE_AntiAliasingMethod::TSR;

	// Post-processing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|PostProcess", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MotionBlurAmount = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics|PostProcess", meta = (ClampMin = "70.0", ClampMax = "120.0"))
	float FieldOfView = 90.0f;

	bool operator == (const FS_GraphicsSettings& Other) const
	{
		const float FloatTol = 1e-4f;

		return ResolutionX == Other.ResolutionX
			&& ResolutionY == Other.ResolutionY
			&& QualityPreset == Other.QualityPreset
			&& FrameRateCap == Other.FrameRateCap
			&& bVSyncEnabled == Other.bVSyncEnabled
			&& bRayTracingEnabled == Other.bRayTracingEnabled
			&& RayTracingQuality == Other.RayTracingQuality
			&& ShadowQuality == Other.ShadowQuality
			&& TextureQuality == Other.TextureQuality
			&& AntiAliasingMethod == Other.AntiAliasingMethod
			&& FMath::IsNearlyEqual(MotionBlurAmount, Other.MotionBlurAmount, FloatTol)
			&& FMath::IsNearlyEqual(FieldOfView, Other.FieldOfView, FloatTol);
	}

	bool operator != (const FS_GraphicsSettings& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * Audio Settings Structure
 */
USTRUCT(BlueprintType)
struct FS_AudioSettings
{
	GENERATED_BODY()

	// Volume Controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AmbientVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UIVolume = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DialogueVolume = 0.9f;

	// Localization & Output
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Localization")
	EE_AudioLanguage CurrentLanguage = EE_AudioLanguage::English;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Output")
	EE_AudioOutput AudioOutput = EE_AudioOutput::Stereo;

	// Accessibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Accessibility")
	bool bSubtitlesEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Accessibility")
	bool bClosedCaptionsEnabled = false;

	bool operator == (const FS_AudioSettings& Other) const
	{
		const float FloatTol = 1e-4f;

		return FMath::IsNearlyEqual(MasterVolume, Other.MasterVolume, FloatTol)
			&& FMath::IsNearlyEqual(SFXVolume, Other.SFXVolume, FloatTol)
			&& FMath::IsNearlyEqual(AmbientVolume, Other.AmbientVolume, FloatTol)
			&& FMath::IsNearlyEqual(UIVolume, Other.UIVolume, FloatTol)
			&& FMath::IsNearlyEqual(MusicVolume, Other.MusicVolume, FloatTol)
			&& FMath::IsNearlyEqual(DialogueVolume, Other.DialogueVolume, FloatTol)
			&& CurrentLanguage == Other.CurrentLanguage
			&& AudioOutput == Other.AudioOutput
			&& bSubtitlesEnabled == bSubtitlesEnabled
			&& bClosedCaptionsEnabled == Other.bClosedCaptionsEnabled;
	}

	bool operator != (const FS_AudioSettings& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * Gameplay Settings Structure
 */
USTRUCT(BlueprintType)
struct FS_GameplaySettings
{
	GENERATED_BODY()

	// Difficulty & Balancing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Difficulty")
	EE_DifficultyLevel DifficultyLevel = EE_DifficultyLevel::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Difficulty", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float SanityDrainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Difficulty", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float EntityDetectionRangeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Sensitivity", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float MouseSensitivity = 1.0;

	// Puzzle & Hints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Puzzle")
	bool bPuzzleHintSystemEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Puzzle", meta = (ClampMin = "10.0", ClampMax = "300.0"))
	float AutoRevealHintTime = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Puzzle")
	bool bAllowSkipPuzzles = false;

	// Gameplay Features
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Features")
	bool bShowObjectiveMarkers = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Features")
	bool bShowInteractionIndicators = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Features")
	bool bAutoPickupItems = false;

	// Comfort Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Comfort")
	float CameraShakeMagnitude = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Comfort", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScreenBlurAmount = 1.0f;

	bool operator == (const FS_GameplaySettings& Other) const
	{
		const float FloatTol = 1e-4f;
		return DifficultyLevel == Other.DifficultyLevel
			&& FMath::IsNearlyEqual(SanityDrainMultiplier, Other.SanityDrainMultiplier, FloatTol)
			&& FMath::IsNearlyEqual(EntityDetectionRangeMultiplier, Other.EntityDetectionRangeMultiplier, FloatTol)
			&& FMath::IsNearlyEqual(MouseSensitivity, Other.MouseSensitivity, FloatTol)
			&& bPuzzleHintSystemEnabled == Other.bPuzzleHintSystemEnabled
			&& FMath::IsNearlyEqual(AutoRevealHintTime, Other.AutoRevealHintTime, FloatTol)
			&& bAllowSkipPuzzles == Other.bAllowSkipPuzzles
			&& bShowObjectiveMarkers == Other.bShowObjectiveMarkers
			&& bShowInteractionIndicators == Other.bShowInteractionIndicators
			&& bAutoPickupItems == Other.bAutoPickupItems
			&& FMath::IsNearlyEqual(CameraShakeMagnitude, Other.CameraShakeMagnitude, FloatTol)
			&& FMath::IsNearlyEqual(ScreenBlurAmount, Other.ScreenBlurAmount, FloatTol);
	}

	bool operator != (const FS_GameplaySettings& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * Control Settings Structure
 */
USTRUCT(BlueprintType)
struct FS_ControlSettings
{
	GENERATED_BODY()

	// Mouse Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Mouse", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Mouse")
	bool bInvertMouseY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Mouse", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CameraZoomSensitivity = 0.8f;

	// Gamepad Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Gamepad", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float GamepadSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Gamepad", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float GamepadDeadzone = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Gamepad")
	bool bInvertGamepadY = false;

	// Gameplay Mechanics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Mechanics")
	bool bAutoSprintEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Mechanics")
	bool bCrouchToggle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Mechanics")
	bool bFlashlightToggle = true;

	// Vibration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Haptics")
	bool bGamepadVibrationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls|Haptics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GamepadVibrationIntensity = 1.0f;

	bool operator == (const FS_ControlSettings& Other) const
	{
		const float FloatTol = 1e-4f;
		return FMath::IsNearlyEqual(MouseSensitivity, Other.MouseSensitivity, FloatTol)
			&& bInvertMouseY == Other.bInvertMouseY
			&& FMath::IsNearlyEqual(CameraZoomSensitivity, Other.CameraZoomSensitivity, FloatTol)
			&& FMath::IsNearlyEqual(GamepadSensitivity, Other.GamepadSensitivity, FloatTol)
			&& FMath::IsNearlyEqual(GamepadDeadzone, Other.GamepadDeadzone, FloatTol)
			&& bInvertGamepadY == Other.bInvertGamepadY
			&& bAutoSprintEnabled == Other.bAutoSprintEnabled
			&& bCrouchToggle == Other.bCrouchToggle
			&& bFlashlightToggle == Other.bFlashlightToggle
			&& bGamepadVibrationEnabled == Other.bGamepadVibrationEnabled
			&& FMath::IsNearlyEqual(GamepadVibrationIntensity, Other.GamepadVibrationIntensity, FloatTol);
	}

	bool operator != (const FS_ControlSettings& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * Accessibility Settings Structure
 */
USTRUCT(BlueprintType)
struct FS_AccessibilitySettings
{
	GENERATED_BODY()

	// Text & UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Text")
	EE_TextSize TextSize = EE_TextSize::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Text")
	EE_TextContrast TextContrast = EE_TextContrast::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Text")
	bool bDyslexiaFontEnabled = false;

	// Vision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Vision")
	EE_ColorBlindMode ColorBlindMode = EE_ColorBlindMode::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Vision")
	bool bHighContrastUIEnabled = false;

	// Motion & Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motion")
	bool bReducedMotionEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Motion")
	EE_PhotosensitivityMode PhotosensitivityMode = EE_PhotosensitivityMode::Off;

	// Audio & Hearing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Audio")
	bool bScreenReaderEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Audio")
	bool bSoundCuesVisualizationEnabled = false;

	// Haptics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Haptics")
	bool bHapticFeedbackEnabled = true;

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Input")
	EE_SingleHandedMode SingleHandedMode = EE_SingleHandedMode::Off;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Input")
	bool bEnableHoldToActivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility|Input", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float HoldActivationTime = 1.0f;

	bool operator == (const FS_AccessibilitySettings& Other) const
	{
		const float FloatTol = 1e-4f;
		return TextSize == Other.TextSize
			&& TextContrast == Other.TextContrast
			&& bDyslexiaFontEnabled == Other.bDyslexiaFontEnabled
			&& ColorBlindMode == Other.ColorBlindMode
			&& bHighContrastUIEnabled == Other.bHighContrastUIEnabled
			&& bReducedMotionEnabled == Other.bReducedMotionEnabled
			&& PhotosensitivityMode == Other.PhotosensitivityMode
			&& bScreenReaderEnabled == Other.bScreenReaderEnabled
			&& bSoundCuesVisualizationEnabled == Other.bSoundCuesVisualizationEnabled
			&& bHapticFeedbackEnabled == Other.bHapticFeedbackEnabled
			&& SingleHandedMode == Other.SingleHandedMode
			&& bEnableHoldToActivate == Other.bEnableHoldToActivate
			&& FMath::IsNearlyEqual(HoldActivationTime, Other.HoldActivationTime, FloatTol);
	}

	bool operator != (const FS_AccessibilitySettings& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * Master Settings Structure - Contains all settings
 */
USTRUCT(BlueprintType)
struct FS_AllSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FS_GraphicsSettings GraphicsSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FS_AudioSettings AudioSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FS_GameplaySettings GameplaySettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FS_ControlSettings ControlSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FS_AccessibilitySettings AccessibilitySettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Metadata")
	int32 SettingsVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Metadata")
	FDateTime LastSavedTime = FDateTime::Now();
};

/**
 * Difficulty Multiplier Structure - For game balance (internal use)
 */
USTRUCT(BlueprintType)
struct FS_DifficultyMultiplier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float SanityDrainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float AIDetectionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float AISpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float EntityChaseMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float BatteryLifeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float PuzzleHintAvailability = 1.0f;
};