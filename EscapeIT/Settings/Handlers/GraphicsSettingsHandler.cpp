#include "EscapeIT/Settings/Handlers/GraphicsSettingsHandler.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "RHI.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformMemory.h"
#include "Kismet/GameplayStatics.h"

static int32 GRHIDeviceMaxTextureMemory;

// ===== MAIN APPLY FUNCTION =====

void FGraphicsSettingsHandler::ApplyToEngine(const FS_GraphicsSettings& Settings, UWorld* World)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("GraphicsSettingsHandler: Failed to get GameUserSettings"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Applying graphics settings..."));

    // Resolution
    SetResolution(Settings.ResolutionX, Settings.ResolutionY);

    // VSync
    SetVSync(Settings.bVSyncEnabled);

    // Frame rate limit
    SetFrameRateLimit(Settings.FrameRateCap);

    // Quality preset
    if (Settings.QualityPreset != EE_GraphicsQuality::Custom)
    {
        SetOverallQuality(Settings.QualityPreset);
    }
    else
    {
        // Custom quality - apply individual settings
        SetTextureQuality(static_cast<int32>(Settings.TextureQuality));
        SetShadowQuality(static_cast<int32>(Settings.ShadowQuality));
        SetAntiAliasing(static_cast<int32>(Settings.AntiAliasingMethod));

        // Post processing based on motion blur
        int32 PostProcessQuality = Settings.MotionBlurAmount > 0.5f ? 3 : 1;
        SetPostProcessing(PostProcessQuality);
    }

    // Ray tracing
    SetRayTracing(Settings.bRayTracingEnabled, Settings.RayTracingQuality, World);

    // Motion blur
    SetMotionBlur(Settings.MotionBlurAmount, World);

    // Field of view
    SetFieldOfView(Settings.FieldOfView, World);

    // Apply all settings
    UserSettings->ApplySettings(false);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Graphics settings applied successfully"));
}

// ===== AUTO-DETECT =====

FS_GraphicsSettings FGraphicsSettingsHandler::AutoDetectSettings()
{
    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Auto-detecting optimal settings..."));

    FS_GraphicsSettings Settings;

    // Detect GPU
    FString GPUName = GRHIAdapterName;
    int32 VRAMSizeMB = GRHIDeviceMaxTextureMemory / (1024 * 1024);
    bool bSupportsRayTracing = GRHISupportsRayTracing;

    // Detect CPU
    int32 CoreCount = FPlatformMisc::NumberOfCores();

    // Detect RAM
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    int32 RAMSizeGB = MemStats.TotalPhysical / (1024 * 1024 * 1024);

    // Calculate performance score (0-100)
    int32 PerformanceScore = 50; // Base score

    // GPU scoring
    if (VRAMSizeMB >= 8192) PerformanceScore += 20;
    else if (VRAMSizeMB >= 4096) PerformanceScore += 10;
    else if (VRAMSizeMB >= 2048) PerformanceScore += 5;

    if (bSupportsRayTracing) PerformanceScore += 15;

    // CPU scoring
    if (CoreCount >= 16) PerformanceScore += 15;
    else if (CoreCount >= 8) PerformanceScore += 10;
    else if (CoreCount >= 6) PerformanceScore += 5;

    // RAM scoring
    if (RAMSizeGB >= 32) PerformanceScore += 10;
    else if (RAMSizeGB >= 16) PerformanceScore += 5;

    PerformanceScore = FMath::Clamp(PerformanceScore, 0, 100);

    UE_LOG(LogTemp, Log, TEXT("  - GPU: %s (VRAM: %d MB)"), *GPUName, VRAMSizeMB);
    UE_LOG(LogTemp, Log, TEXT("  - CPU: %d cores"), CoreCount);
    UE_LOG(LogTemp, Log, TEXT("  - RAM: %d GB"), RAMSizeGB);
    UE_LOG(LogTemp, Log, TEXT("  - Performance Score: %d"), PerformanceScore);

    // Determine quality preset based on score
    EE_GraphicsQuality RecommendedQuality;
    if (PerformanceScore >= 85)
        RecommendedQuality = EE_GraphicsQuality::Ultra;
    else if (PerformanceScore >= 70)
        RecommendedQuality = EE_GraphicsQuality::High;
    else if (PerformanceScore >= 50)
        RecommendedQuality = EE_GraphicsQuality::Medium;
    else
        RecommendedQuality = EE_GraphicsQuality::Low;

    // Set preset
    Settings.QualityPreset = RecommendedQuality;

    // Set resolution based on VRAM
    if (VRAMSizeMB >= 8192)
    {
        Settings.ResolutionX = 3840;
        Settings.ResolutionY = 2160; // 4K
    }
    else if (VRAMSizeMB >= 4096)
    {
        Settings.ResolutionX = 2560;
        Settings.ResolutionY = 1440; // 1440p
    }
    else
    {
        Settings.ResolutionX = 1920;
        Settings.ResolutionY = 1080; // 1080p
    }

    // Enable ray tracing only if supported and high-end GPU
    Settings.bRayTracingEnabled = bSupportsRayTracing && PerformanceScore >= 75;
    Settings.RayTracingQuality = Settings.bRayTracingEnabled ? EE_RayTracingQuality::High : EE_RayTracingQuality::Low;

    // VSync - enable for better consistency
    Settings.bVSyncEnabled = true;

    // Frame rate cap - uncapped for high-end, capped for low-end
    Settings.FrameRateCap = PerformanceScore >= 70 ? 0 : 60;

    // FOV
    Settings.FieldOfView = 90.0f;

    // Motion blur
    Settings.MotionBlurAmount = PerformanceScore >= 70 ? 0.5f : 0.0f;

    // Individual quality settings based on preset
    switch (RecommendedQuality)
    {
    case EE_GraphicsQuality::Ultra:
        Settings.TextureQuality = EE_TextureQuality::Epic;
        Settings.ShadowQuality = EE_ShadowQuality::Epic;
        Settings.AntiAliasingMethod = EE_AntiAliasingMethod::TAA;
        break;
    case EE_GraphicsQuality::High:
        Settings.TextureQuality = EE_TextureQuality::High;
        Settings.ShadowQuality = EE_ShadowQuality::High;
        Settings.AntiAliasingMethod = EE_AntiAliasingMethod::MSAA_4x;
        break;
    case EE_GraphicsQuality::Medium:
        Settings.TextureQuality = EE_TextureQuality::Medium;
        Settings.ShadowQuality = EE_ShadowQuality::Medium;
        Settings.AntiAliasingMethod = EE_AntiAliasingMethod::FXAA;
        break;
    case EE_GraphicsQuality::Low:
        Settings.TextureQuality = EE_TextureQuality::Low;
        Settings.ShadowQuality = EE_ShadowQuality::Low;
        Settings.AntiAliasingMethod = EE_AntiAliasingMethod::None;
        break;
    default:
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("  - Recommended Quality: %d"), static_cast<int32>(RecommendedQuality));
    UE_LOG(LogTemp, Log, TEXT("  - Resolution: %dx%d"), Settings.ResolutionX, Settings.ResolutionY);
    UE_LOG(LogTemp, Log, TEXT("  - Ray Tracing: %s"), Settings.bRayTracingEnabled ? TEXT("Enabled") : TEXT("Disabled"));

    return Settings;
}

// ===== INDIVIDUAL SETTERS =====

void FGraphicsSettingsHandler::SetResolution(int32 X, int32 Y)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    FIntPoint Resolution(X, Y);
    UserSettings->SetScreenResolution(Resolution);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Resolution set to %dx%d"), X, Y);
}

void FGraphicsSettingsHandler::SetVSync(bool bEnabled)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    UserSettings->SetVSyncEnabled(bEnabled);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: VSync %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void FGraphicsSettingsHandler::SetFrameRateLimit(int32 FrameRate)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    float Limit = (FrameRate == 0) ? 0.0f : static_cast<float>(FrameRate);
    UserSettings->SetFrameRateLimit(Limit);

    if (FrameRate == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Frame rate uncapped"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Frame rate limited to %d"), FrameRate);
    }
}

void FGraphicsSettingsHandler::SetOverallQuality(EE_GraphicsQuality Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    int32 QualityLevel = static_cast<int32>(Quality);
    UserSettings->SetOverallScalabilityLevel(QualityLevel);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Overall quality set to %d"), QualityLevel);
}

void FGraphicsSettingsHandler::SetRayTracing(bool bEnabled, EE_RayTracingQuality Quality, UWorld* World)
{
    if (!GRHISupportsRayTracing)
    {
        if (bEnabled)
        {
            UE_LOG(LogTemp, Warning, TEXT("GraphicsSettingsHandler: Ray tracing not supported on this hardware"));
        }
        return;
    }

    if (bEnabled)
    {
        ExecuteConsoleCommand(World, TEXT("r.RayTracing 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections 1"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Shadows 1"));

        // Quality settings
        switch (Quality)
        {
        case EE_RayTracingQuality::Low:
            ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination.SamplesPerPixel 1"));
            ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections.SamplesPerPixel 1"));
            break;
        case EE_RayTracingQuality::Medium:
            ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination.SamplesPerPixel 2"));
            ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections.SamplesPerPixel 2"));
            break;
        case EE_RayTracingQuality::High:
            ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination.SamplesPerPixel 4"));
            ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections.SamplesPerPixel 4"));
            break;
        case EE_RayTracingQuality::Epic:
            ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination.SamplesPerPixel 8"));
            ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections.SamplesPerPixel 8"));
            break;
        }

        UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Ray tracing enabled (Quality: %d)"), static_cast<int32>(Quality));
    }
    else
    {
        ExecuteConsoleCommand(World, TEXT("r.RayTracing 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.GlobalIllumination 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Reflections 0"));
        ExecuteConsoleCommand(World, TEXT("r.RayTracing.Shadows 0"));

        UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Ray tracing disabled"));
    }
}

void FGraphicsSettingsHandler::SetMotionBlur(float Amount, UWorld* World)
{
    Amount = FMath::Clamp(Amount, 0.0f, 1.0f);

    if (Amount > 0.0f)
    {
        ExecuteConsoleCommand(World, TEXT("r.MotionBlurQuality 4"));

        FString Command = FString::Printf(TEXT("r.MotionBlur.Amount %.2f"), Amount);
        ExecuteConsoleCommand(World, Command);

        UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Motion blur set to %.2f"), Amount);
    }
    else
    {
        ExecuteConsoleCommand(World, TEXT("r.MotionBlurQuality 0"));
        UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Motion blur disabled"));
    }
}

void FGraphicsSettingsHandler::SetFieldOfView(float FOV, UWorld* World)
{
    FOV = FMath::Clamp(FOV, 70.0f, 120.0f);

    FString Command = FString::Printf(TEXT("fov %.1f"), FOV);
    ExecuteConsoleCommand(World, Command);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: FOV set to %.1f"), FOV);
}

// ===== CUSTOM QUALITY SETTINGS =====

void FGraphicsSettingsHandler::SetViewDistance(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    Quality = FMath::Clamp(Quality, 0, 3);
    UserSettings->SetViewDistanceQuality(Quality);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: View distance quality set to %d"), Quality);
}

void FGraphicsSettingsHandler::SetShadowQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    Quality = FMath::Clamp(Quality, 0, 3);
    UserSettings->SetShadowQuality(Quality);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Shadow quality set to %d"), Quality);
}

void FGraphicsSettingsHandler::SetTextureQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    Quality = FMath::Clamp(Quality, 0, 3);
    UserSettings->SetTextureQuality(Quality);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Texture quality set to %d"), Quality);
}

void FGraphicsSettingsHandler::SetAntiAliasing(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    Quality = FMath::Clamp(Quality, 0, 3);
    UserSettings->SetAntiAliasingQuality(Quality);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Anti-aliasing quality set to %d"), Quality);
}

void FGraphicsSettingsHandler::SetPostProcessing(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    Quality = FMath::Clamp(Quality, 0, 3);
    UserSettings->SetPostProcessingQuality(Quality);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Post-processing quality set to %d"), Quality);
}

void FGraphicsSettingsHandler::SetEffectsQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    Quality = FMath::Clamp(Quality, 0, 3);
    UserSettings->SetVisualEffectQuality(Quality);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Effects quality set to %d"), Quality);
}

void FGraphicsSettingsHandler::SetFoliageQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    Quality = FMath::Clamp(Quality, 0, 3);
    UserSettings->SetFoliageQuality(Quality);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Foliage quality set to %d"), Quality);
}

void FGraphicsSettingsHandler::SetShadingQuality(int32 Quality)
{
    UGameUserSettings* UserSettings = GetGameUserSettings();
    if (!UserSettings) return;

    Quality = FMath::Clamp(Quality, 0, 3);
    UserSettings->SetShadingQuality(Quality);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Shading quality set to %d"), Quality);
}

// ===== HELPER FUNCTIONS =====

UGameUserSettings* FGraphicsSettingsHandler::GetGameUserSettings()
{
    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();

    if (!UserSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("GraphicsSettingsHandler: Failed to get GameUserSettings"));
    }

    return UserSettings;
}

void FGraphicsSettingsHandler::ExecuteConsoleCommand(UWorld* World, const FString& Command)
{
    if (!World || !GEngine)
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphicsSettingsHandler: Cannot execute command '%s' - World or Engine is null"), *Command);
        return;
    }

    GEngine->Exec(World, *Command);
}