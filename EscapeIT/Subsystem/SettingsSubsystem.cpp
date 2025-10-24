#include "SettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeIT/SaveGames/SettingsSaveGame.h"
#include "GameFramework/GameUserSettings.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

void USettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSettingsFromSlot();
}

void USettingsSubsystem::Deinitialize()
{
	SaveSettingsToSlot();
	Super::Deinitialize();
}

bool USettingsSubsystem::ApplyAllSettings(const FS_AllSettings& NewSettings)
{
	bool bSuccess = true;
	bool bAnySettingApplied = false;

	// Validate settings first
	if (!ValidateSettings(NewSettings))
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Settings validation failed"));
		return false;
	}

	// Store old settings for rollback if needed
	FS_AllSettings OldSettings = AllSettings;

	// Update the settings data
	AllSettings = NewSettings;

	// Apply Graphics Settings
	if (AllSettings.GraphicsSettings != OldSettings.GraphicsSettings)
	{
		ApplyGraphicsSettingsToEngine(AllSettings.GraphicsSettings);
		OnGraphicsSettingsChanged.Broadcast(AllSettings.GraphicsSettings);
		bAnySettingApplied = true;
		UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Graphics settings applied"));
	}

	// Apply Audio Settings
	if (AllSettings.AudioSettings != OldSettings.AudioSettings)
	{
		ApplyAudioSettingsToEngine(AllSettings.AudioSettings);
		OnAudioSettingsChanged.Broadcast(AllSettings.AudioSettings);
		bAnySettingApplied = true;
		UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Audio settings applied"));
	}

	// Apply Gameplay Settings
	if (AllSettings.GameplaySettings != OldSettings.GameplaySettings)
	{
		ApplyGameplaySettingsToEngine(AllSettings.GameplaySettings);
		OnGameplaySettingsChanged.Broadcast(AllSettings.GameplaySettings);
		bAnySettingApplied = true;
		UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Gameplay settings applied"));
	}

	// Apply Control Settings
	if (AllSettings.ControlSettings != OldSettings.ControlSettings)
	{
		ApplyControlSettingsToEngine(AllSettings.ControlSettings);
		OnControlSettingsChanged.Broadcast(AllSettings.ControlSettings);
		bAnySettingApplied = true;
		UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Control settings applied"));
	}

	// Apply Accessibility Settings
	if (AllSettings.AccessibilitySettings != OldSettings.AccessibilitySettings)
	{
		ApplyAccessibilitySettingsToEngine(AllSettings.AccessibilitySettings);
		OnAccessibilitySettingsChanged.Broadcast(AllSettings.AccessibilitySettings);
		bAnySettingApplied = true;
		UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Accessibility settings applied"));
	}

	// Broadcast all settings changed if any setting was applied
	if (bAnySettingApplied)
	{
		OnAllSettingsChanged.Broadcast(AllSettings);
		UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: All settings changes broadcasted"));
	}

	// Save to disk
	if (!SaveAllSettings())
	{
		UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Settings applied but failed to save to disk"));
		// Don't return false here - settings are applied even if save failed
	}

	UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: All settings applied successfully"));
	return true;
}

bool USettingsSubsystem::ValidateSettings(const FS_AllSettings& Settings) const
{
	bool bValid = true;

	// Validate Graphics Settings
	if (Settings.GraphicsSettings.ResolutionX < 640 || Settings.GraphicsSettings.ResolutionX > 7680)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid resolution width: %d (must be 640-7680)"),
			Settings.GraphicsSettings.ResolutionX);
		bValid = false;
	}

	if (Settings.GraphicsSettings.ResolutionY < 480 || Settings.GraphicsSettings.ResolutionY > 4320)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid resolution height: %d (must be 480-4320)"),
			Settings.GraphicsSettings.ResolutionY);
		bValid = false;
	}

	if (Settings.GraphicsSettings.FrameRateCap < 30 || Settings.GraphicsSettings.FrameRateCap > 300)
	{
		UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Unusual frame rate cap: %d"),
			Settings.GraphicsSettings.FrameRateCap);
		// Warning only, don't fail validation
	}

	if (Settings.GraphicsSettings.FieldOfView < 60.0f || Settings.GraphicsSettings.FieldOfView > 120.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid FOV: %.2f (must be 60-120)"),
			Settings.GraphicsSettings.FieldOfView);
		bValid = false;
	}

	if (Settings.GraphicsSettings.MotionBlurAmount < 0.0f || Settings.GraphicsSettings.MotionBlurAmount > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid motion blur: %.2f (must be 0-1)"),
			Settings.GraphicsSettings.MotionBlurAmount);
		bValid = false;
	}

	// Validate Audio Settings
	if (Settings.AudioSettings.MasterVolume < 0.0f || Settings.AudioSettings.MasterVolume > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid master volume: %.2f (must be 0-1)"),
			Settings.AudioSettings.MasterVolume);
		bValid = false;
	}

	if (Settings.AudioSettings.MusicVolume < 0.0f || Settings.AudioSettings.MusicVolume > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid music volume: %.2f (must be 0-1)"),
			Settings.AudioSettings.MusicVolume);
		bValid = false;
	}

	if (Settings.AudioSettings.SFXVolume < 0.0f || Settings.AudioSettings.SFXVolume > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid SFX volume: %.2f (must be 0-1)"),
			Settings.AudioSettings.SFXVolume);
		bValid = false;
	}

	if (Settings.AudioSettings.AmbientVolume < 0.0f || Settings.AudioSettings.AmbientVolume > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid ambient volume: %.2f (must be 0-1)"),
			Settings.AudioSettings.AmbientVolume);
		bValid = false;
	}

	if (Settings.AudioSettings.UIVolume < 0.0f || Settings.AudioSettings.UIVolume > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid UI volume: %.2f (must be 0-1)"),
			Settings.AudioSettings.UIVolume);
		bValid = false;
	}

	if (Settings.AudioSettings.DialogueVolume < 0.0f || Settings.AudioSettings.DialogueVolume > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid dialogue volume: %.2f (must be 0-1)"),
			Settings.AudioSettings.DialogueVolume);
		bValid = false;
	}

	// Validate Gameplay Settings
	if (Settings.GameplaySettings.MouseSensitivity <= 0.0f || Settings.GameplaySettings.MouseSensitivity > 5.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid mouse sensitivity: %.2f (must be 0-5)"),
			Settings.GameplaySettings.MouseSensitivity);
		bValid = false;
	}

	if (Settings.GameplaySettings.SanityDrainMultiplier < 0.0f || Settings.GameplaySettings.SanityDrainMultiplier > 3.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid sanity drain multiplier: %.2f (must be 0-3)"),
			Settings.GameplaySettings.SanityDrainMultiplier);
		bValid = false;
	}

	if (Settings.GameplaySettings.EntityDetectionRangeMultiplier < 0.0f || Settings.GameplaySettings.EntityDetectionRangeMultiplier > 3.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid detection range multiplier: %.2f (must be 0-3)"),
			Settings.GameplaySettings.EntityDetectionRangeMultiplier);
		bValid = false;
	}

	if (Settings.GameplaySettings.AutoRevealHintTime < 0.0f || Settings.GameplaySettings.AutoRevealHintTime > 600.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid hint reveal time: %.2f (must be 0-600)"),
			Settings.GameplaySettings.AutoRevealHintTime);
		bValid = false;
	}

	if (Settings.GameplaySettings.CameraShakeMagnitude < 0.0f || Settings.GameplaySettings.CameraShakeMagnitude > 2.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid camera shake: %.2f (must be 0-2)"),
			Settings.GameplaySettings.CameraShakeMagnitude);
		bValid = false;
	}

	if (Settings.GameplaySettings.ScreenBlurAmount < 0.0f || Settings.GameplaySettings.ScreenBlurAmount > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid screen blur: %.2f (must be 0-1)"),
			Settings.GameplaySettings.ScreenBlurAmount);
		bValid = false;
	}

	// Validate Control Settings
	if (Settings.ControlSettings.MouseSensitivity <= 0.0f || Settings.ControlSettings.MouseSensitivity > 5.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid control mouse sensitivity: %.2f (must be 0-5)"),
			Settings.ControlSettings.MouseSensitivity);
		bValid = false;
	}

	if (Settings.ControlSettings.GamepadSensitivity <= 0.0f || Settings.ControlSettings.GamepadSensitivity > 5.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid gamepad sensitivity: %.2f (must be 0-5)"),
			Settings.ControlSettings.GamepadSensitivity);
		bValid = false;
	}

	if (Settings.ControlSettings.GamepadDeadzone < 0.0f || Settings.ControlSettings.GamepadDeadzone > 0.9f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid gamepad deadzone: %.2f (must be 0-0.9)"),
			Settings.ControlSettings.GamepadDeadzone);
		bValid = false;
	}

	if (Settings.ControlSettings.CameraZoomSensitivity < 0.0f || Settings.ControlSettings.CameraZoomSensitivity > 2.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid zoom sensitivity: %.2f (must be 0-2)"),
			Settings.ControlSettings.CameraZoomSensitivity);
		bValid = false;
	}

	if (Settings.ControlSettings.GamepadVibrationIntensity < 0.0f || Settings.ControlSettings.GamepadVibrationIntensity > 1.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid vibration intensity: %.2f (must be 0-1)"),
			Settings.ControlSettings.GamepadVibrationIntensity);
		bValid = false;
	}

	// Validate Accessibility Settings
	if (Settings.AccessibilitySettings.HoldActivationTime < 0.1f || Settings.AccessibilitySettings.HoldActivationTime > 10.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Invalid hold activation time: %.2f (must be 0.1-10)"),
			Settings.AccessibilitySettings.HoldActivationTime);
		bValid = false;
	}

	if (!bValid)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Settings validation failed - see errors above"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Settings validation passed"));
	}

	return bValid;
}

bool USettingsSubsystem::SaveAllSettings()
{
	USettingsSaveGame* Save = Cast<USettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(USettingsSaveGame::StaticClass())
	);

	if (!Save)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to create save game object"));
		return false;
	}

	Save->AllSettings = AllSettings;
	Save->LastSavedTime = FDateTime::UtcNow();

	bool bSaveSuccess = UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, SaveUserIndex);

	if (bSaveSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Settings saved successfully to slot '%s'"), *SaveSlotName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to save settings to slot '%s'"), *SaveSlotName);
	}

	return bSaveSuccess;
}

void USettingsSubsystem::ResetSettingsToDefault()
{
	// Create default settings
	FS_AllSettings DefaultSettings;

	// Apply defaults
	ApplyAllSettings(DefaultSettings);

	// Save to slot
	SaveSettingsToSlot();
}

void USettingsSubsystem::SaveSettingsToSlot()
{
	SaveAllSettings();
}

void USettingsSubsystem::LoadSettingsFromSlot()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		USettingsSaveGame* Save = Cast<USettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
		if (Save)
		{
			AllSettings = Save->AllSettings;

			// Apply loaded settings
			ApplyGraphicsSettingsToEngine(AllSettings.GraphicsSettings);
			ApplyAudioSettingsToEngine(AllSettings.AudioSettings);
			ApplyGameplaySettingsToEngine(AllSettings.GameplaySettings);
			ApplyControlSettingsToEngine(AllSettings.ControlSettings);
			ApplyAccessibilitySettingsToEngine(AllSettings.AccessibilitySettings);

			OnAllSettingsChanged.Broadcast(AllSettings);
		}
	}
}

// ===== GRAPHICS SETTINGS =====
void USettingsSubsystem::InitGraphicSettings()
{
	// LOW PRESET
	FS_GraphicsSettings LowPreset;
	LowPreset.QualityPreset = EE_GraphicsQuality::Low;
	LowPreset.ResolutionX = 1280;
	LowPreset.ResolutionY = 720;
	LowPreset.bVSyncEnabled = false;
	LowPreset.FrameRateCap = 60;
	LowPreset.bRayTracingEnabled = false;
	LowPreset.RayTracingQuality = EE_RayTracingQuality::Low;
	LowPreset.ShadowQuality = EE_ShadowQuality::Low;
	LowPreset.TextureQuality = EE_TextureQuality::Low;
	LowPreset.AntiAliasingMethod = EE_AntiAliasingMethod::None;
	LowPreset.MotionBlurAmount = 0.0f;
	LowPreset.FieldOfView = 90.0f;
	GraphicsPresets.Add(EE_GraphicsQuality::Low, LowPreset);

	// MEDIUM PRESET
	FS_GraphicsSettings MediumPreset;
	MediumPreset.QualityPreset = EE_GraphicsQuality::Medium;
	MediumPreset.ResolutionX = 1920;
	MediumPreset.ResolutionY = 1080;
	MediumPreset.bVSyncEnabled = false;
	MediumPreset.FrameRateCap = 60;
	MediumPreset.bRayTracingEnabled = false;
	MediumPreset.RayTracingQuality = EE_RayTracingQuality::Low;
	MediumPreset.ShadowQuality = EE_ShadowQuality::Medium;
	MediumPreset.TextureQuality = EE_TextureQuality::Medium;
	MediumPreset.AntiAliasingMethod = EE_AntiAliasingMethod::FXAA;
	MediumPreset.MotionBlurAmount = 0.25f;
	MediumPreset.FieldOfView = 90.0f;
	GraphicsPresets.Add(EE_GraphicsQuality::Medium, MediumPreset);

	// HIGH PRESET
	FS_GraphicsSettings HighPreset;
	HighPreset.QualityPreset = EE_GraphicsQuality::High;
	HighPreset.ResolutionX = 1920;
	HighPreset.ResolutionY = 1080;
	HighPreset.bVSyncEnabled = true;
	HighPreset.FrameRateCap = 60;
	HighPreset.bRayTracingEnabled = true;
	HighPreset.RayTracingQuality = EE_RayTracingQuality::Medium;
	HighPreset.ShadowQuality = EE_ShadowQuality::High;
	HighPreset.TextureQuality = EE_TextureQuality::High;
	HighPreset.AntiAliasingMethod = EE_AntiAliasingMethod::TAA;
	HighPreset.MotionBlurAmount = 0.5f;
	HighPreset.FieldOfView = 90.0f;
	GraphicsPresets.Add(EE_GraphicsQuality::High, HighPreset);

	// ULTRA PRESET
	FS_GraphicsSettings UltraPreset;
	UltraPreset.QualityPreset = EE_GraphicsQuality::Ultra;
	UltraPreset.ResolutionX = 3840;
	UltraPreset.ResolutionY = 2160;
	UltraPreset.bVSyncEnabled = true;
	UltraPreset.FrameRateCap = 0; // Unlimited
	UltraPreset.bRayTracingEnabled = true;
	UltraPreset.RayTracingQuality = EE_RayTracingQuality::High;
	UltraPreset.ShadowQuality = EE_ShadowQuality::Epic;
	UltraPreset.TextureQuality = EE_TextureQuality::Epic;
	UltraPreset.AntiAliasingMethod = EE_AntiAliasingMethod::MSAA_4x;
	UltraPreset.MotionBlurAmount = 1.0f;
	UltraPreset.FieldOfView = 90.0f;
	GraphicsPresets.Add(EE_GraphicsQuality::Ultra, UltraPreset);

	// CUSTOM PRESET (empty - user will fill)
	FS_GraphicsSettings CustomPreset;
	CustomPreset.QualityPreset = EE_GraphicsQuality::Custom;
	GraphicsPresets.Add(EE_GraphicsQuality::Custom, CustomPreset);

	UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Graphics presets initialized"));
}

void USettingsSubsystem::ApplyGraphicsSettings(const FS_GraphicsSettings& NewSettings)
{
	AllSettings.GraphicsSettings = NewSettings;
	ApplyGraphicsSettingsToEngine(NewSettings);
	OnGraphicsSettingsChanged.Broadcast(NewSettings);
	SaveSettingsToSlot();
}

FS_GraphicsSettings USettingsSubsystem::GetGraphicsPreset(EE_GraphicsQuality GraphicsQuality) const
{
	if (const FS_GraphicsSettings* Found = GraphicsPresets.Find(GraphicsQuality))
	{
		return *Found; // trả copy (ok)
	}

	// Nếu không tìm thấy, trả preset mặc định (hoặc Low)
	if (GraphicsPresets.Num() > 0)
	{
		return GraphicsPresets.CreateConstIterator()->Value;
	}

	return FS_GraphicsSettings();
}

void USettingsSubsystem::ApplyGraphicsSettingsToEngine(const FS_GraphicsSettings& Settings)
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to get GameUserSettings"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Applying graphics settings to engine"));

	// ===== RESOLUTION =====
	FIntPoint CurrentResolution = UserSettings->GetScreenResolution();
	FIntPoint NewResolution(Settings.ResolutionX, Settings.ResolutionY);

	if (CurrentResolution != NewResolution)
	{
		UserSettings->SetScreenResolution(NewResolution);
		UE_LOG(LogTemp, Log, TEXT("  - Resolution: %dx%d"), NewResolution.X, NewResolution.Y);
	}

	// ===== WINDOW MODE =====
	// You might want to expose this in your settings struct
	EWindowMode::Type WindowMode = UserSettings->GetFullscreenMode();
	// UserSettings->SetFullscreenMode(EWindowMode::Fullscreen); // or Windowed, WindowedFullscreen

	// ===== VSYNC =====
	bool bCurrentVSync = UserSettings->IsVSyncEnabled();
	if (bCurrentVSync != Settings.bVSyncEnabled)
	{
		UserSettings->SetVSyncEnabled(Settings.bVSyncEnabled);
		UE_LOG(LogTemp, Log, TEXT("  - VSync: %s"), Settings.bVSyncEnabled ? TEXT("Enabled") : TEXT("Disabled"));
	}

	// ===== FRAME RATE LIMIT =====
	float CurrentFrameRateLimit = UserSettings->GetFrameRateLimit();
	float NewFrameRateLimit = (Settings.FrameRateCap == 0) ? 0.0f : static_cast<float>(Settings.FrameRateCap);

	if (!FMath::IsNearlyEqual(CurrentFrameRateLimit, NewFrameRateLimit, 0.1f))
	{
		UserSettings->SetFrameRateLimit(NewFrameRateLimit);
		if (Settings.FrameRateCap == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("  - Frame Rate: Unlimited"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("  - Frame Rate: %d FPS"), Settings.FrameRateCap);
		}
	}

	// ===== OVERALL QUALITY PRESET =====
	// Only apply preset if not Custom
	if (Settings.QualityPreset != EE_GraphicsQuality::Custom)
	{
		int32 QualityLevel = static_cast<int32>(Settings.QualityPreset);
		int32 CurrentQualityLevel = UserSettings->GetOverallScalabilityLevel();

		if (CurrentQualityLevel != QualityLevel)
		{
			UserSettings->SetOverallScalabilityLevel(QualityLevel);
			UE_LOG(LogTemp, Log, TEXT("  - Overall Quality: Level %d"), QualityLevel);
		}
	}
	else
	{
		// Custom preset - apply individual settings
		UE_LOG(LogTemp, Log, TEXT("  - Quality: Custom"));

		// ===== VIEW DISTANCE =====
		int32 ViewDistanceQuality = static_cast<int32>(Settings.ShadowQuality); // Or create separate setting
		if (UserSettings->GetViewDistanceQuality() != ViewDistanceQuality)
		{
			UserSettings->SetViewDistanceQuality(ViewDistanceQuality);
			UE_LOG(LogTemp, Log, TEXT("    - View Distance: %d"), ViewDistanceQuality);
		}

		// ===== SHADOW QUALITY =====
		int32 ShadowQuality = static_cast<int32>(Settings.ShadowQuality);
		if (UserSettings->GetShadowQuality() != ShadowQuality)
		{
			UserSettings->SetShadowQuality(ShadowQuality);
			UE_LOG(LogTemp, Log, TEXT("    - Shadow Quality: %d"), ShadowQuality);
		}

		// ===== TEXTURE QUALITY =====
		int32 TextureQuality = static_cast<int32>(Settings.TextureQuality);
		if (UserSettings->GetTextureQuality() != TextureQuality)
		{
			UserSettings->SetTextureQuality(TextureQuality);
			UE_LOG(LogTemp, Log, TEXT("    - Texture Quality: %d"), TextureQuality);
		}

		// ===== ANTI-ALIASING =====
		int32 AntiAliasingQuality = static_cast<int32>(Settings.AntiAliasingMethod);
		if (UserSettings->GetAntiAliasingQuality() != AntiAliasingQuality)
		{
			UserSettings->SetAntiAliasingQuality(AntiAliasingQuality);
			UE_LOG(LogTemp, Log, TEXT("    - Anti-Aliasing: %d"), AntiAliasingQuality);
		}

		// ===== POST PROCESSING =====
		// Motion blur is part of post-processing quality
		int32 PostProcessQuality = (Settings.MotionBlurAmount > 0.5f) ? 3 : 1;
		if (UserSettings->GetPostProcessingQuality() != PostProcessQuality)
		{
			UserSettings->SetPostProcessingQuality(PostProcessQuality);
			UE_LOG(LogTemp, Log, TEXT("    - Post Processing: %d (Motion Blur: %.2f)"),
				PostProcessQuality, Settings.MotionBlurAmount);
		}

		// ===== EFFECTS QUALITY =====
		int32 EffectsQuality = static_cast<int32>(Settings.TextureQuality); // Or create separate setting
		if (UserSettings->GetVisualEffectQuality() != EffectsQuality)
		{
			UserSettings->SetVisualEffectQuality(EffectsQuality);
			UE_LOG(LogTemp, Log, TEXT("    - Effects Quality: %d"), EffectsQuality);
		}

		// ===== FOLIAGE QUALITY =====
		int32 FoliageQuality = static_cast<int32>(Settings.ShadowQuality); // Or create separate setting
		if (UserSettings->GetFoliageQuality() != FoliageQuality)
		{
			UserSettings->SetFoliageQuality(FoliageQuality);
			UE_LOG(LogTemp, Log, TEXT("    - Foliage Quality: %d"), FoliageQuality);
		}

		// ===== SHADING QUALITY =====
		int32 ShadingQuality = static_cast<int32>(Settings.TextureQuality);
		if (UserSettings->GetShadingQuality() != ShadingQuality)
		{
			UserSettings->SetShadingQuality(ShadingQuality);
			UE_LOG(LogTemp, Log, TEXT("    - Shading Quality: %d"), ShadingQuality);
		}
	}

	// ===== RAY TRACING =====
	// Note: Ray tracing settings are typically handled via console variables
	// as they're not part of UGameUserSettings
	if (Settings.bRayTracingEnabled)
	{
		// Enable ray tracing features via console variables
		if (GEngine)
		{
			// Ray Tracing Global Illumination
			GEngine->Exec(GetWorld(), TEXT("r.RayTracing.GlobalIllumination 1"));

			// Ray Tracing Reflections
			GEngine->Exec(GetWorld(), TEXT("r.RayTracing.Reflections 1"));

			// Ray Tracing Shadows
			GEngine->Exec(GetWorld(), TEXT("r.RayTracing.Shadows 1"));

			// Ray Tracing Ambient Occlusion
			GEngine->Exec(GetWorld(), TEXT("r.RayTracing.AmbientOcclusion 1"));

			// Ray Tracing Quality based on setting
			int32 RTQuality = static_cast<int32>(Settings.RayTracingQuality);
			FString RTQualityCmd = FString::Printf(TEXT("r.RayTracing.SamplesPerPixel %d"),
				FMath::Clamp(RTQuality + 1, 1, 4));
			GEngine->Exec(GetWorld(), *RTQualityCmd);

			UE_LOG(LogTemp, Log, TEXT("  - Ray Tracing: Enabled (Quality: %d)"), RTQuality);
		}
	}
	else
	{
		// Disable ray tracing
		if (GEngine)
		{
			GEngine->Exec(GetWorld(), TEXT("r.RayTracing.GlobalIllumination 0"));
			GEngine->Exec(GetWorld(), TEXT("r.RayTracing.Reflections 0"));
			GEngine->Exec(GetWorld(), TEXT("r.RayTracing.Shadows 0"));
			GEngine->Exec(GetWorld(), TEXT("r.RayTracing.AmbientOcclusion 0"));

			UE_LOG(LogTemp, Log, TEXT("  - Ray Tracing: Disabled"));
		}
	}

	// ===== MOTION BLUR =====
	if (GEngine)
	{
		// Motion blur amount (0-1)
		FString MotionBlurCmd = FString::Printf(TEXT("r.MotionBlurQuality %.2f"),
			Settings.MotionBlurAmount * 4.0f); // Scale 0-1 to 0-4 for UE
		GEngine->Exec(GetWorld(), *MotionBlurCmd);

		// Disable motion blur entirely if amount is 0
		if (Settings.MotionBlurAmount <= 0.0f)
		{
			GEngine->Exec(GetWorld(), TEXT("r.MotionBlur.Max 0"));
		}
		else
		{
			GEngine->Exec(GetWorld(), TEXT("r.MotionBlur.Max 1"));
		}
	}

	// ===== FIELD OF VIEW =====
	// FOV is typically handled per-camera/player, not globally
	// Store it for later application to player camera
	// You'll need to implement this in your player controller or camera manager
	if (GEngine)
	{
		// Save to a custom console variable or handle in player controller
		FString FOVCmd = FString::Printf(TEXT("fov %.1f"), Settings.FieldOfView);
		GEngine->Exec(GetWorld(), *FOVCmd);
		UE_LOG(LogTemp, Log, TEXT("  - Field of View: %.1f"), Settings.FieldOfView);
	}

	// ===== APPLY SETTINGS =====
	// Apply immediately without saving to config file yet
	UserSettings->ApplySettings(false);

	UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Graphics settings applied to engine"));
}

void USettingsSubsystem::SetGraphicsQuality(EE_GraphicsQuality QualityPreset)
{
	// Ensure presets are initialized
	if (GraphicsPresets.Num() == 0)
	{
		InitGraphicSettings();
	}

	// Get the preset
	const FS_GraphicsSettings* PresetPtr = GraphicsPresets.Find(QualityPreset);
	if (!PresetPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Quality preset %d not found"),
			static_cast<int32>(QualityPreset));
		return;
	}

	// Copy preset to current settings
	AllSettings.GraphicsSettings = *PresetPtr;

	// Apply to engine
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);

	UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Graphics quality set to preset %d"),
		static_cast<int32>(QualityPreset));
}

void USettingsSubsystem::SetFrameRateCap(int32 FrameRate)
{
	AllSettings.GraphicsSettings.FrameRateCap = FrameRate;
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetRayTracingEnabled(bool bEnabled)
{
	AllSettings.GraphicsSettings.bRayTracingEnabled = bEnabled;
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetRayTracingQuality(EE_RayTracingQuality Quality)
{
	AllSettings.GraphicsSettings.RayTracingQuality = Quality;
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetShadowQuality(EE_ShadowQuality Quality)
{
	AllSettings.GraphicsSettings.ShadowQuality = Quality;
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetTextureQuality(EE_TextureQuality Quality)
{
	AllSettings.GraphicsSettings.TextureQuality = Quality;
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetAntiAliasingMethod(EE_AntiAliasingMethod Method)
{
	AllSettings.GraphicsSettings.AntiAliasingMethod = Method;
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetMotionBlurAmount(float Amount)
{
	AllSettings.GraphicsSettings.MotionBlurAmount = FMath::Clamp(Amount, 0.0f, 1.0f);
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetResolution(int32 X, int32 Y)
{
	AllSettings.GraphicsSettings.ResolutionX = X;
	AllSettings.GraphicsSettings.ResolutionY = Y;
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetVSyncEnabled(bool bEnabled)
{
	AllSettings.GraphicsSettings.bVSyncEnabled = bEnabled;
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

void USettingsSubsystem::SetFieldOfView(float FOV)
{
	AllSettings.GraphicsSettings.FieldOfView = FMath::Clamp(FOV, 70.0f, 120.0f);
	ApplyGraphicsSettings(AllSettings.GraphicsSettings);
}

// ===== AUDIO SETTINGS =====

void USettingsSubsystem::ApplyAudioSettings(const FS_AudioSettings& NewSettings)
{
	AllSettings.AudioSettings = NewSettings;
	ApplyAudioSettingsToEngine(NewSettings);
	OnAudioSettingsChanged.Broadcast(NewSettings);
	SaveSettingsToSlot();
}

void USettingsSubsystem::ApplyAudioSettingsToEngine(const FS_AudioSettings& Settings)
{
	if (GEngine && GEngine->GetMainAudioDevice())
	{
		//FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice();

		// Note: Unreal uses Sound Classes and Sound Mixes for volume control
		// You'll need to create Sound Classes for each category and use Sound Mix to adjust them

		// This is a simplified example - in production you'd use Sound Mix Modifiers
		// Example implementation would look like:
		/*
		USoundMix* SoundMix = LoadObject<USoundMix>(nullptr, TEXT("/Game/Audio/MasterSoundMix"));
		if (SoundMix)
		{
			AudioDevice->PushSoundMixModifier(SoundMix);
			// Set individual class volumes based on Settings
		}
		*/
	}
}

void USettingsSubsystem::SetMasterVolume(float Volume)
{
	AllSettings.AudioSettings.MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetSFXVolume(float Volume)
{
	AllSettings.AudioSettings.SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetAmbientVolume(float Volume)
{
	AllSettings.AudioSettings.AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetUIVolume(float Volume)
{
	AllSettings.AudioSettings.UIVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetMusicVolume(float Volume)
{
	AllSettings.AudioSettings.MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetDialogueVolume(float Volume)
{
	AllSettings.AudioSettings.DialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetAudioLanguage(EE_AudioLanguage Language)
{
	AllSettings.AudioSettings.CurrentLanguage = Language;
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetAudioOutput(EE_AudioOutput Output)
{
	AllSettings.AudioSettings.AudioOutput = Output;
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetClosedCaptionsEnabled(bool bEnabled)
{
	AllSettings.AudioSettings.bClosedCaptionsEnabled = bEnabled;
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetSubtitlesEnabled(bool bEnabled)
{
	AllSettings.AudioSettings.bSubtitlesEnabled = bEnabled;
	ApplyAudioSettings(AllSettings.AudioSettings);
}

void USettingsSubsystem::SetMouseSensitivity(float Sensitivity)
{
	AllSettings.ControlSettings.MouseSensitivity = FMath::Clamp(Sensitivity, 0.1f, 3.0f);
	ApplyControlSettings(AllSettings.ControlSettings);
}

// ===== GAMEPLAY SETTINGS =====

void USettingsSubsystem::ApplyGameplaySettings(const FS_GameplaySettings& NewSettings)
{
	AllSettings.GameplaySettings = NewSettings;
	ApplyGameplaySettingsToEngine(NewSettings);
	OnGameplaySettingsChanged.Broadcast(NewSettings);
	SaveSettingsToSlot();
}

void USettingsSubsystem::ApplyGameplaySettingsToEngine(const FS_GameplaySettings& Settings)
{
	// Gameplay settings are typically handled by game-specific systems
	// Broadcast the change so other systems can react
	// For example, the sanity system would listen to these changes
}

void USettingsSubsystem::SetDifficultyLevel(EE_DifficultyLevel Difficulty)
{
	AllSettings.GameplaySettings.DifficultyLevel = Difficulty;

	// Auto-adjust multipliers based on difficulty
	FS_DifficultyMultiplier Multipliers = GetDifficultyMultiplier(Difficulty);
	AllSettings.GameplaySettings.SanityDrainMultiplier = Multipliers.SanityDrainMultiplier;
	AllSettings.GameplaySettings.EntityDetectionRangeMultiplier = Multipliers.AIDetectionMultiplier;

	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetSanityDrainMultiplier(float Multiplier)
{
	AllSettings.GameplaySettings.SanityDrainMultiplier = FMath::Clamp(Multiplier, 0.5f, 2.0f);
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetEntityDetectionRangeMultiplier(float Multiplier)
{
	AllSettings.GameplaySettings.EntityDetectionRangeMultiplier = FMath::Clamp(Multiplier, 0.5f, 2.0f);
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetPuzzleHintSystemEnabled(bool bEnabled)
{
	AllSettings.GameplaySettings.bPuzzleHintSystemEnabled = bEnabled;
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetAutoRevealHintTime(float Time)
{
	AllSettings.GameplaySettings.AutoRevealHintTime = FMath::Clamp(Time, 10.0f, 300.0f);
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetAllowSkipPuzzles(bool bAllow)
{
	AllSettings.GameplaySettings.bAllowSkipPuzzles = bAllow;
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetShowObjectiveMarkers(bool bShow)
{
	AllSettings.GameplaySettings.bShowObjectiveMarkers = bShow;
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetShowInteractionIndicators(bool bShow)
{
	AllSettings.GameplaySettings.bShowInteractionIndicators = bShow;
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetAutoPickupItems(bool bEnabled)
{
	AllSettings.GameplaySettings.bAutoPickupItems = bEnabled;
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetCameraShakeMagnitude(float Magnitude)
{
	AllSettings.GameplaySettings.CameraShakeMagnitude = FMath::Clamp(Magnitude, 0.0f, 2.0f);
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

void USettingsSubsystem::SetScreenBlurAmount(float Amount)
{
	AllSettings.GameplaySettings.ScreenBlurAmount = FMath::Clamp(Amount, 0.0f, 1.0f);
	ApplyGameplaySettings(AllSettings.GameplaySettings);
}

FS_DifficultyMultiplier USettingsSubsystem::GetDifficultyMultiplier(EE_DifficultyLevel Difficulty)
{
	FS_DifficultyMultiplier Multiplier;

	switch (Difficulty)
	{
	case EE_DifficultyLevel::Easy:
		Multiplier.SanityDrainMultiplier = 0.7f;
		Multiplier.AIDetectionMultiplier = 0.7f;
		Multiplier.AISpeedMultiplier = 0.8f;
		Multiplier.EntityChaseMultiplier = 0.8f;
		Multiplier.BatteryLifeMultiplier = 1.5f;
		Multiplier.PuzzleHintAvailability = 1.5f;
		break;

	case EE_DifficultyLevel::Normal:
		Multiplier.SanityDrainMultiplier = 1.0f;
		Multiplier.AIDetectionMultiplier = 1.0f;
		Multiplier.AISpeedMultiplier = 1.0f;
		Multiplier.EntityChaseMultiplier = 1.0f;
		Multiplier.BatteryLifeMultiplier = 1.0f;
		Multiplier.PuzzleHintAvailability = 1.0f;
		break;

	case EE_DifficultyLevel::Hard:
		Multiplier.SanityDrainMultiplier = 1.3f;
		Multiplier.AIDetectionMultiplier = 1.3f;
		Multiplier.AISpeedMultiplier = 1.2f;
		Multiplier.EntityChaseMultiplier = 1.2f;
		Multiplier.BatteryLifeMultiplier = 0.7f;
		Multiplier.PuzzleHintAvailability = 0.7f;
		break;

	case EE_DifficultyLevel::Nightmare:
		Multiplier.SanityDrainMultiplier = 1.7f;
		Multiplier.AIDetectionMultiplier = 1.5f;
		Multiplier.AISpeedMultiplier = 1.4f;
		Multiplier.EntityChaseMultiplier = 1.5f;
		Multiplier.BatteryLifeMultiplier = 0.5f;
		Multiplier.PuzzleHintAvailability = 0.5f;
		break;
	}

	return Multiplier;
}

// ===== CONTROL SETTINGS =====

void USettingsSubsystem::ApplyControlSettings(const FS_ControlSettings& NewSettings)
{
	AllSettings.ControlSettings = NewSettings;
	ApplyControlSettingsToEngine(NewSettings);
	OnControlSettingsChanged.Broadcast(NewSettings);
	SaveSettingsToSlot();
}

void USettingsSubsystem::ApplyControlSettingsToEngine(const FS_ControlSettings& Settings)
{
	// Control settings are typically handled by the player controller/character
	// The player controller would listen to OnControlSettingsChanged and adjust accordingly
}

void USettingsSubsystem::SetInvertMouseY(bool bInvert)
{
	AllSettings.ControlSettings.bInvertMouseY = bInvert;
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetCameraZoomSensitivity(float Sensitivity)
{
	AllSettings.ControlSettings.CameraZoomSensitivity = FMath::Clamp(Sensitivity, 0.0f, 1.0f);
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetGamepadSensitivity(float Sensitivity)
{
	AllSettings.ControlSettings.GamepadSensitivity = FMath::Clamp(Sensitivity, 0.1f, 3.0f);
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetGamepadDeadzone(float Deadzone)
{
	AllSettings.ControlSettings.GamepadDeadzone = FMath::Clamp(Deadzone, 0.0f, 0.5f);
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetInvertGamepadY(bool bInvert)
{
	AllSettings.ControlSettings.bInvertGamepadY = bInvert;
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetAutoSprintEnabled(bool bEnabled)
{
	AllSettings.ControlSettings.bAutoSprintEnabled = bEnabled;
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetCrouchToggle(bool bToggle)
{
	AllSettings.ControlSettings.bCrouchToggle = bToggle;
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetFlashlightToggle(bool bToggle)
{
	AllSettings.ControlSettings.bFlashlightToggle = bToggle;
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetGamepadVibrationEnabled(bool bEnabled)
{
	AllSettings.ControlSettings.bGamepadVibrationEnabled = bEnabled;
	ApplyControlSettings(AllSettings.ControlSettings);
}

void USettingsSubsystem::SetGamepadVibrationIntensity(float Intensity)
{
	AllSettings.ControlSettings.GamepadVibrationIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	ApplyControlSettings(AllSettings.ControlSettings);
}

// ===== ACCESSIBILITY SETTINGS =====

void USettingsSubsystem::ApplyAccessibilitySettings(const FS_AccessibilitySettings& NewSettings)
{
	AllSettings.AccessibilitySettings = NewSettings;
	ApplyAccessibilitySettingsToEngine(NewSettings);
	OnAccessibilitySettingsChanged.Broadcast(NewSettings);
	SaveSettingsToSlot();
}

void USettingsSubsystem::ApplyAccessibilitySettingsToEngine(const FS_AccessibilitySettings& Settings)
{
	// Accessibility settings are typically handled by UI and various game systems
	// UI widgets would listen to these changes and adjust their appearance
}

void USettingsSubsystem::SetTextSize(EE_TextSize Size)
{
	AllSettings.AccessibilitySettings.TextSize = Size;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetTextContrast(EE_TextContrast Contrast)
{
	AllSettings.AccessibilitySettings.TextContrast = Contrast;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetDyslexiaFontEnabled(bool bEnabled)
{
	AllSettings.AccessibilitySettings.bDyslexiaFontEnabled = bEnabled;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetColorBlindMode(EE_ColorBlindMode Mode)
{
	AllSettings.AccessibilitySettings.ColorBlindMode = Mode;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetHighContrastUIEnabled(bool bEnabled)
{
	AllSettings.AccessibilitySettings.bHighContrastUIEnabled = bEnabled;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetReducedMotionEnabled(bool bEnabled)
{
	AllSettings.AccessibilitySettings.bReducedMotionEnabled = bEnabled;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetPhotosensitivityMode(EE_PhotosensitivityMode Mode)
{
	AllSettings.AccessibilitySettings.PhotosensitivityMode = Mode;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetScreenReaderEnabled(bool bEnabled)
{
	AllSettings.AccessibilitySettings.bScreenReaderEnabled = bEnabled;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetSoundCuesVisualizationEnabled(bool bEnabled)
{
	AllSettings.AccessibilitySettings.bSoundCuesVisualizationEnabled = bEnabled;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetHapticFeedbackEnabled(bool bEnabled)
{
	AllSettings.AccessibilitySettings.bHapticFeedbackEnabled = bEnabled;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetSingleHandedMode(EE_SingleHandedMode Mode)
{
	AllSettings.AccessibilitySettings.SingleHandedMode = Mode;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetEnableHoldToActivate(bool bEnabled)
{
	AllSettings.AccessibilitySettings.bEnableHoldToActivate = bEnabled;
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}

void USettingsSubsystem::SetHoldActivationTime(float Time)
{
	AllSettings.AccessibilitySettings.HoldActivationTime = FMath::Clamp(Time, 0.1f, 5.0f);
	ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
}