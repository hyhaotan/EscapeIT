
#include "GraphicsSettingsHandler.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/PlayerController.h"

UClass* DummyForInclude = nullptr; // tránh unused-warning nếu cần

// Helper: get game user settings
UGameUserSettings* FGraphicsSettingsHandler::GetGameUserSettings()
{
    return GEngine ? UGameUserSettings::GetGameUserSettings() : nullptr;
}

void FGraphicsSettingsHandler::ExecuteConsoleCommand(UWorld* World, const FString& Command)
{
    if (GEngine)
    {
        // If we have a world, pass it. Otherwise pass nullptr.
        GEngine->Exec(World, *Command);
    }
}

// Apply full settings to engine (main entry)
void FGraphicsSettingsHandler::ApplyToEngine(const FS_GraphicsSettings& Settings, UWorld* World)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("FGraphicsSettingsHandler: Failed to get GameUserSettings"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("FGraphicsSettingsHandler: Applying graphics settings to engine"));

    // Resolution
    FIntPoint CurrentRes = UserSettings->GetScreenResolution();
    FIntPoint NewRes(Settings.ResolutionX, Settings.ResolutionY);
    if (CurrentRes != NewRes)
    {
        UserSettings->SetScreenResolution(NewRes);
        UE_LOG(LogTemp, Log, TEXT("  - Resolution: %dx%d"), NewRes.X, NewRes.Y);
    }

    // VSync
    bool bCurrentVSync = UserSettings->IsVSyncEnabled();
    if (bCurrentVSync != Settings.bVSyncEnabled)
    {
        UserSettings->SetVSyncEnabled(Settings.bVSyncEnabled);
        UE_LOG(LogTemp, Log, TEXT("  - VSync: %s"), Settings.bVSyncEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    }

    // Frame rate limit
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

    // Overall quality (when not Custom)
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
        // Custom: apply per-quality values
        int32 ViewDistanceQuality = static_cast<int32>(Settings.ShadowQuality); // reuse shadow as placeholder
        if (UserSettings->GetViewDistanceQuality() != ViewDistanceQuality)
        {
            UserSettings->SetViewDistanceQuality(ViewDistanceQuality);
            UE_LOG(LogTemp, Log, TEXT("    - View Distance: %d"), ViewDistanceQuality);
        }

        int32 ShadowQuality = static_cast<int32>(Settings.ShadowQuality);
        if (UserSettings->GetShadowQuality() != ShadowQuality)
        {
            UserSettings->SetShadowQuality(ShadowQuality);
            UE_LOG(LogTemp, Log, TEXT("    - Shadow Quality: %d"), ShadowQuality);
        }

        int32 TextureQuality = static_cast<int32>(Settings.TextureQuality);
        if (UserSettings->GetTextureQuality() != TextureQuality)
        {
            UserSettings->SetTextureQuality(TextureQuality);
            UE_LOG(LogTemp, Log, TEXT("    - Texture Quality: %d"), TextureQuality);
        }

        int32 AntiAliasingQuality = static_cast<int32>(Settings.AntiAliasingMethod);
        if (UserSettings->GetAntiAliasingQuality() != AntiAliasingQuality)
        {
            UserSettings->SetAntiAliasingQuality(AntiAliasingQuality);
            UE_LOG(LogTemp, Log, TEXT("    - Anti-Aliasing: %d"), AntiAliasingQuality);
        }

        int32 PostProcessQuality = (Settings.MotionBlurAmount > 0.5f) ? 3 : 1;
        if (UserSettings->GetPostProcessingQuality() != PostProcessQuality)
        {
            UserSettings->SetPostProcessingQuality(PostProcessQuality);
            UE_LOG(LogTemp, Log, TEXT("    - Post Processing: %d (Motion Blur: %.2f)"),
                PostProcessQuality, Settings.MotionBlurAmount);
        }

        int32 EffectsQuality = static_cast<int32>(Settings.TextureQuality);
        if (UserSettings->GetVisualEffectQuality() != EffectsQuality)
        {
            UserSettings->SetVisualEffectQuality(EffectsQuality);
            UE_LOG(LogTemp, Log, TEXT("    - Effects Quality: %d"), EffectsQuality);
        }

        int32 FoliageQuality = static_cast<int32>(Settings.ShadowQuality);
        if (UserSettings->GetFoliageQuality() != FoliageQuality)
        {
            UserSettings->SetFoliageQuality(FoliageQuality);
            UE_LOG(LogTemp, Log, TEXT("    - Foliage Quality: %d"), FoliageQuality);
        }

        int32 ShadingQuality = static_cast<int32>(Settings.TextureQuality);
        if (UserSettings->GetShadingQuality() != ShadingQuality)
        {
            UserSettings->SetShadingQuality(ShadingQuality);
            UE_LOG(LogTemp, Log, TEXT("    - Shading Quality: %d"), ShadingQuality);
        }
    }

    // Ray tracing (console vars)
    if (Settings.bRayTracingEnabled)
    {
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Shadows 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.AmbientOcclusion 1"));

        int32 RTQuality = static_cast<int32>(Settings.RayTracingQuality);
        FString RTQualityCmd = FString::Printf(TEXT("r.RayTracing.SamplesPerPixel %d"), FMath::Clamp(RTQuality + 1, 1, 4));
        ExecuteConsoleCommand(World, RTQualityCmd);

        UE_LOG(LogTemp, Log, TEXT("  - Ray Tracing: Enabled (Quality: %d)"), RTQuality);
    }
    else
    {
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Shadows 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.AmbientOcclusion 0"));

        UE_LOG(LogTemp, Log, TEXT("  - Ray Tracing: Disabled"));
    }

    // Motion blur
    {
        FString MotionBlurCmd = FString::Printf(TEXT("r.MotionBlurQuality %.2f"), Settings.MotionBlurAmount * 4.0f);
        ExecuteConsoleCommand(World, MotionBlurCmd);

        if (Settings.MotionBlurAmount <= 0.0f)
            ExecuteConsoleCommand(World, TEXT("r.MotionBlur.Max 0"));
        else
            ExecuteConsoleCommand(World, TEXT("r.MotionBlur.Max 1"));
    }

    // Field of view (global console var approach)
    {
        FString FOVCmd = FString::Printf(TEXT("fov %.1f"), Settings.FieldOfView);
        ExecuteConsoleCommand(World, FOVCmd);
        UE_LOG(LogTemp, Log, TEXT("  - Field of View: %.1f"), Settings.FieldOfView);
    }

    // Finally apply UGameUserSettings changes
    UserSettings->ApplySettings(false);

    UE_LOG(LogTemp, Log, TEXT("FGraphicsSettingsHandler: Graphics settings applied"));
}

// Auto-detect a reasonable settings object from current engine/usersettings
FS_GraphicsSettings FGraphicsSettingsHandler::AutoDetectSettings()
{
    FS_GraphicsSettings Out;
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings)
    {
        // fallback defaults
        Out.ResolutionX = 1920;
        Out.ResolutionY = 1080;
        Out.QualityPreset = EE_GraphicsQuality::Medium;
        return Out;
    }

    // Use current scalability and resolution to pick a preset
    int32 Scalability = UserSettings->GetOverallScalabilityLevel();
    FIntPoint Res = UserSettings->GetScreenResolution();

    Out.ResolutionX = Res.X;
    Out.ResolutionY = Res.Y;

    if (Scalability <= 1)
    {
        Out.QualityPreset = EE_GraphicsQuality::Low;
        Out.ShadowQuality = EE_ShadowQuality::Low;
        Out.TextureQuality = EE_TextureQuality::Low;
        Out.AntiAliasingMethod = EE_AntiAliasingMethod::None;
    }
    else if (Scalability == 2)
    {
        Out.QualityPreset = EE_GraphicsQuality::Medium;
        Out.ShadowQuality = EE_ShadowQuality::Medium;
        Out.TextureQuality = EE_TextureQuality::Medium;
        Out.AntiAliasingMethod = EE_AntiAliasingMethod::FXAA;
    }
    else if (Scalability == 3)
    {
        Out.QualityPreset = EE_GraphicsQuality::High;
        Out.ShadowQuality = EE_ShadowQuality::High;
        Out.TextureQuality = EE_TextureQuality::High;
        Out.AntiAliasingMethod = EE_AntiAliasingMethod::TAA;
    }
    else
    {
        Out.QualityPreset = EE_GraphicsQuality::Ultra;
        Out.ShadowQuality = EE_ShadowQuality::Epic;
        Out.TextureQuality = EE_TextureQuality::Epic;
        Out.AntiAliasingMethod = EE_AntiAliasingMethod::MSAA_4x;
    }

    // Frame rate / vSync defaults
    Out.FrameRateCap = static_cast<int32>(UserSettings->GetFrameRateLimit());
    Out.bVSyncEnabled = UserSettings->IsVSyncEnabled();

    // conservative defaults for other fields
    Out.bRayTracingEnabled = false;
    Out.MotionBlurAmount = (UserSettings->GetPostProcessingQuality() > 1) ? 0.5f : 0.0f;
    Out.FieldOfView = 90.0f;

    return Out;
}

// Individual setters - thin wrappers around UGameUserSettings / console commands
void FGraphicsSettingsHandler::SetResolution(int32 X, int32 Y)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetScreenResolution(FIntPoint(X, Y));
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetVSync(bool bEnabled)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetVSyncEnabled(bEnabled);
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetFrameRateLimit(int32 FrameRate)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetFrameRateLimit(static_cast<float>(FrameRate));
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetOverallQuality(EE_GraphicsQuality Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetOverallScalabilityLevel(static_cast<int32>(Quality));
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetRayTracing(bool bEnabled, EE_RayTracingQuality Quality, UWorld* World)
{
    if (bEnabled)
    {
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Shadows 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.AmbientOcclusion 1"));

        int32 RTQuality = static_cast<int32>(Quality);
        FString RTQualityCmd = FString::Printf(TEXT("r.RayTracing.SamplesPerPixel %d"), FMath::Clamp(RTQuality + 1, 1, 4));
        ExecuteConsoleCommand(World, RTQualityCmd);
    }
    else
    {
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Shadows 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.AmbientOcclusion 0"));
    }
}

void FGraphicsSettingsHandler::SetMotionBlur(float Amount, UWorld* World)
{
    FString MotionBlurCmd = FString::Printf(TEXT("r.MotionBlurQuality %.2f"), FMath::Clamp(Amount, 0.0f, 1.0f) * 4.0f);
    ExecuteConsoleCommand(World, MotionBlurCmd);

    if (Amount <= 0.0f)
        ExecuteConsoleCommand(World, TEXT("r.MotionBlur.Max 0"));
    else
        ExecuteConsoleCommand(World, TEXT("r.MotionBlur.Max 1"));
}

void FGraphicsSettingsHandler::SetFieldOfView(float FOV, UWorld* World)
{
    FString FOVCmd = FString::Printf(TEXT("fov %.1f"), FMath::Clamp(FOV, 70.0f, 120.0f));
    ExecuteConsoleCommand(World, FOVCmd);
}

void FGraphicsSettingsHandler::SetViewDistance(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetViewDistanceQuality(Quality);
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetShadowQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetShadowQuality(Quality);
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetTextureQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetTextureQuality(Quality);
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetAntiAliasing(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetAntiAliasingQuality(Quality);
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetPostProcessing(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetPostProcessingQuality(Quality);
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetEffectsQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetVisualEffectQuality(Quality);
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetFoliageQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetFoliageQuality(Quality);
        UserSettings->ApplySettings(false);
    }
}

void FGraphicsSettingsHandler::SetShadingQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetShadingQuality(Quality);
        UserSettings->ApplySettings(false);
    }
}
