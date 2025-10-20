#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EscapeIT/Data/EscapeITSettingsStructs.h"
#include "SettingsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllSettingsChanged, const FS_AllSettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGraphicsSettingsChanged, const FS_GraphicsSettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioSettingsChanged, const FS_AudioSettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplaySettingsChanged, const FS_GameplaySettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnControlSettingsChanged, const FS_ControlSettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccessibilitySettingsChanged, const FS_AccessibilitySettings&, NewSettings);

UCLASS()
class ESCAPEIT_API USettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Initialization & Cleanup
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ===== General Settings Management =====

	/** Apply all settings at once */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAllSettings(const FS_AllSettings& NewSettings);

	/** Get all current settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	FS_AllSettings GetAllSettings() const { return AllSettings; }

	/** Reset all settings to defaults */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ResetSettingsToDefault();

	/** Save settings to slot */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettingsToSlot();

	/** Load settings from slot */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettingsFromSlot();

	// ===== Graphics Settings =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void ApplyGraphicsSettings(const FS_GraphicsSettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	FS_GraphicsSettings GetGraphicsSettings() const { return AllSettings.GraphicsSettings; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetResolution(int32 X, int32 Y);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetGraphicsQuality(EE_GraphicsQuality QualityPreset);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetFrameRateCap(int32 FrameRate);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetVSyncEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetRayTracingEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetRayTracingQuality(EE_RayTracingQuality Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetShadowQuality(EE_ShadowQuality Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetTextureQuality(EE_TextureQuality Quality);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetAntiAliasingMethod(EE_AntiAliasingMethod Method);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetMotionBlurAmount(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetFieldOfView(float FOV);

	// ===== Audio Settings =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void ApplyAudioSettings(const FS_AudioSettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	FS_AudioSettings GetAudioSettings() const { return AllSettings.AudioSettings; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetAmbientVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetUIVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetDialogueVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetAudioLanguage(EE_AudioLanguage Language);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetAudioOutput(EE_AudioOutput Output);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetSubtitlesEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetClosedCaptionsEnabled(bool bEnabled);

	// ===== Gameplay Settings =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void ApplyGameplaySettings(const FS_GameplaySettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	FS_GameplaySettings GetGameplaySettings() const { return AllSettings.GameplaySettings; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetDifficultyLevel(EE_DifficultyLevel Difficulty);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetSanityDrainMultiplier(float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetEntityDetectionRangeMultiplier(float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetPuzzleHintSystemEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetAutoRevealHintTime(float Time);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetAllowSkipPuzzles(bool bAllow);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetShowObjectiveMarkers(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetShowInteractionIndicators(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetAutoPickupItems(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetCameraShakeMagnitude(float Magnitude);

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetScreenBlurAmount(float Amount);

	// ===== Control Settings =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void ApplyControlSettings(const FS_ControlSettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	FS_ControlSettings GetControlSettings() const { return AllSettings.ControlSettings; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetMouseSensitivity(float Sensitivity);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetInvertMouseY(bool bInvert);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetCameraZoomSensitivity(float Sensitivity);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetGamepadSensitivity(float Sensitivity);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetGamepadDeadzone(float Deadzone);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetInvertGamepadY(bool bInvert);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetAutoSprintEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetCrouchToggle(bool bToggle);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetFlashlightToggle(bool bToggle);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetGamepadVibrationEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetGamepadVibrationIntensity(float Intensity);

	// ===== Accessibility Settings =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void ApplyAccessibilitySettings(const FS_AccessibilitySettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	FS_AccessibilitySettings GetAccessibilitySettings() const { return AllSettings.AccessibilitySettings; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetTextSize(EE_TextSize Size);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetTextContrast(EE_TextContrast Contrast);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetDyslexiaFontEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetColorBlindMode(EE_ColorBlindMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetHighContrastUIEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetReducedMotionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetPhotosensitivityMode(EE_PhotosensitivityMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetScreenReaderEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetSoundCuesVisualizationEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetHapticFeedbackEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetSingleHandedMode(EE_SingleHandedMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetEnableHoldToActivate(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Accessibility")
	void SetHoldActivationTime(float Time);

	// ===== Difficulty Multipliers (Internal Use) =====

	UFUNCTION(BlueprintCallable, Category = "Settings|Difficulty")
	FS_DifficultyMultiplier GetDifficultyMultiplier(EE_DifficultyLevel Difficulty);

	// ===== Delegates =====

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnAllSettingsChanged OnAllSettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnGraphicsSettingsChanged OnGraphicsSettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnAudioSettingsChanged OnAudioSettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnGameplaySettingsChanged OnGameplaySettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnControlSettingsChanged OnControlSettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnAccessibilitySettingsChanged OnAccessibilitySettingsChanged;

protected:
	// ===== Internal Methods =====

	/** Apply graphics settings to the engine */
	void ApplyGraphicsSettingsToEngine(const FS_GraphicsSettings& Settings);

	/** Apply audio settings to the audio system */
	void ApplyAudioSettingsToEngine(const FS_AudioSettings& Settings);

	/** Apply gameplay settings to the game */
	void ApplyGameplaySettingsToEngine(const FS_GameplaySettings& Settings);

	/** Apply control settings to input system */
	void ApplyControlSettingsToEngine(const FS_ControlSettings& Settings);

	/** Apply accessibility settings to UI and systems */
	void ApplyAccessibilitySettingsToEngine(const FS_AccessibilitySettings& Settings);

	// ===== Members =====

	UPROPERTY()
	FS_AllSettings AllSettings;

	UPROPERTY()
	FString SaveSlotName = TEXT("GameSettings");

	UPROPERTY()
	uint32 SaveUserIndex = 0;
};