#include "EscapeIT/Settings/Handlers/GraphicsSettingsHandler.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "RHI.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformMemory.h"
#include "Kismet/GameplayStatics.h"

// ===== STATIC MEMBERS =====
TMap<EE_GraphicsQuality, FS_GraphicsSettings> FGraphicsSettingsHandler::GraphicsPresets;
FS_GraphicsSettings FGraphicsSettingsHandler::CustomPreset;
bool FGraphicsSettingsHandler::bPresetsInitialized = false;

// ===== INITIALIZATION =====

void FGraphicsSettingsHandler::Initialize()
{
    if (!bPresetsInitialized)
    {
        InitializePresets();
        bPresetsInitialized = true;
    }
}

void FGraphicsSettingsHandler::InitializePresets()
{
    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Initializing presets..."));

    GraphicsPresets.Empty();

    // Low preset
    FS_GraphicsSettings LowPreset;
    LowPreset.QualityPreset = EE_GraphicsQuality::Low;
    LowPreset.ResolutionX = 1280;
    LowPreset.ResolutionY = 720;
    LowPreset.bVSyncEnabled = false;
    LowPreset.FrameRateCap = 60;
    LowPreset.TextureQuality = EE_TextureQuality::Low;
    LowPreset.ShadowQuality = EE_ShadowQuality::Low;
    LowPreset.AntiAliasingMethod = EE_AntiAliasingMethod::None;
    LowPreset.MotionBlurAmount = 0.0f;
    LowPreset.bRayTracingEnabled = false;
    LowPreset.FieldOfView = 90.0f;
    GraphicsPresets.Add(EE_GraphicsQuality::Low, LowPreset);

    // Medium preset
    FS_GraphicsSettings MediumPreset;
    MediumPreset.QualityPreset = EE_GraphicsQuality::Medium;
    MediumPreset.ResolutionX = 1920;
    MediumPreset.ResolutionY = 1080;
    MediumPreset.bVSyncEnabled = true;
    MediumPreset.FrameRateCap = 0;
    MediumPreset.TextureQuality = EE_TextureQuality::Medium;
    MediumPreset.ShadowQuality = EE_ShadowQuality::Medium;
    MediumPreset.AntiAliasingMethod = EE_AntiAliasingMethod::FXAA;
    MediumPreset.MotionBlurAmount = 0.2f;
    MediumPreset.bRayTracingEnabled = false;
    MediumPreset.FieldOfView = 90.0f;
    GraphicsPresets.Add(EE_GraphicsQuality::Medium, MediumPreset);

    // High preset
    FS_GraphicsSettings HighPreset;
    HighPreset.QualityPreset = EE_GraphicsQuality::High;
    HighPreset.ResolutionX = 2560;
    HighPreset.ResolutionY = 1440;
    HighPreset.bVSyncEnabled = true;
    HighPreset.FrameRateCap = 0;
    HighPreset.TextureQuality = EE_TextureQuality::High;
    HighPreset.ShadowQuality = EE_ShadowQuality::High;
    HighPreset.AntiAliasingMethod = EE_AntiAliasingMethod::MSAA_4x;
    HighPreset.MotionBlurAmount = 0.4f;
    HighPreset.bRayTracingEnabled = false;
    HighPreset.FieldOfView = 90.0f;
    GraphicsPresets.Add(EE_GraphicsQuality::High, HighPreset);

    // Ultra preset
    FS_GraphicsSettings UltraPreset;
    UltraPreset.QualityPreset = EE_GraphicsQuality::Ultra;
    UltraPreset.ResolutionX = 3840;
    UltraPreset.ResolutionY = 2160;
    UltraPreset.bVSyncEnabled = true;
    UltraPreset.FrameRateCap = 0;
    UltraPreset.TextureQuality = EE_TextureQuality::Epic;
    UltraPreset.ShadowQuality = EE_ShadowQuality::Epic;
    UltraPreset.AntiAliasingMethod = EE_AntiAliasingMethod::TAA;
    UltraPreset.MotionBlurAmount = 1.0f;
    UltraPreset.bRayTracingEnabled = true;
    UltraPreset.RayTracingQuality = EE_RayTracingQuality::High;
    UltraPreset.FieldOfView = 90.0f;
    GraphicsPresets.Add(EE_GraphicsQuality::Ultra, UltraPreset);

    // Custom preset (default values)
    CustomPreset = MediumPreset;
    CustomPreset.QualityPreset = EE_GraphicsQuality::Custom;
    GraphicsPresets.Add(EE_GraphicsQuality::Custom, CustomPreset);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Presets initialized successfully"));
}

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
    FString GPUName = GetGPUName();
    int32 VRAMSizeMB = GetAvailableVRAM();
    bool bSupportsRayTracing = IsRayTracingSupported();

    // Detect CPU
    int32 CoreCount = FPlatformMisc::NumberOfCores();

    // Detect RAM
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    int32 RAMSizeGB = MemStats.TotalPhysical / (1024 * 1024 * 1024);

    // Calculate performance score
    int32 PerformanceScore = CalculateGPUPerformanceScore();

    // Add CPU scoring
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

    // Get preset and customize based on hardware
    Settings = GetPreset(RecommendedQuality);

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

    UE_LOG(LogTemp, Log, TEXT("  - Recommended Quality: %d"), static_cast<int32>(RecommendedQuality));
    UE_LOG(LogTemp, Log, TEXT("  - Resolution: %dx%d"), Settings.ResolutionX, Settings.ResolutionY);
    UE_LOG(LogTemp, Log, TEXT("  - Ray Tracing: %s"), Settings.bRayTracingEnabled ? TEXT("Enabled") : TEXT("Disabled"));

    return Settings;
}

// ===== VALIDATION =====

bool FGraphicsSettingsHandler::ValidateSettings(const FS_GraphicsSettings& Settings,
    TArray<FString>& OutErrors, TArray<FString>& OutWarnings)
{
    bool bValid = true;

    // Resolution validation
    if (Settings.ResolutionX < 640 || Settings.ResolutionY < 480)
    {
        OutErrors.Add(TEXT("Resolution is too low (minimum 640x480)"));
        bValid = false;
    }

    if (Settings.ResolutionX > 7680 || Settings.ResolutionY > 4320)
    {
        OutErrors.Add(TEXT("Resolution is too high (maximum 7680x4320)"));
        bValid = false;
    }

    // Check if resolution is supported
    TArray<FIntPoint> SupportedResolutions = GetAvailableResolutions();
    FIntPoint RequestedRes(Settings.ResolutionX, Settings.ResolutionY);
    if (!SupportedResolutions.Contains(RequestedRes))
    {
        OutWarnings.Add(FString::Printf(TEXT("Resolution %dx%d may not be supported by your display"),
            Settings.ResolutionX, Settings.ResolutionY));
    }

    // Frame rate validation
    if (Settings.FrameRateCap < 0 || Settings.FrameRateCap > 300)
    {
        OutErrors.Add(TEXT("Frame rate cap must be between 0 and 300"));
        bValid = false;
    }

    // FOV validation
    if (Settings.FieldOfView < 70.0f || Settings.FieldOfView > 120.0f)
    {
        OutErrors.Add(TEXT("Field of view must be between 70 and 120"));
        bValid = false;
    }

    // Motion blur validation
    if (Settings.MotionBlurAmount < 0.0f || Settings.MotionBlurAmount > 1.0f)
    {
        OutErrors.Add(TEXT("Motion blur amount must be between 0.0 and 1.0"));
        bValid = false;
    }

    // Ray tracing validation
    if (Settings.bRayTracingEnabled && !IsRayTracingSupported())
    {
        OutErrors.Add(TEXT("Ray tracing is not supported on this hardware"));
        bValid = false;
    }

    // VRAM check
    int32 EstimatedVRAM = EstimateVRAMUsage(Settings);
    int32 AvailableVRAM = GetAvailableVRAM();

    if (EstimatedVRAM > AvailableVRAM)
    {
        OutWarnings.Add(FString::Printf(
            TEXT("Settings may exceed available VRAM (%d MB required, %d MB available)"),
            EstimatedVRAM, AvailableVRAM));
    }

    return bValid;
}

int32 FGraphicsSettingsHandler::EstimateVRAMUsage(const FS_GraphicsSettings& Settings)
{
    // Rough estimation based on settings
    int32 BaseUsage = 512; // Base game usage

    // Resolution impact
    int32 PixelCount = Settings.ResolutionX * Settings.ResolutionY;
    BaseUsage += (PixelCount / 1000000) * 100; // ~100MB per megapixel

    // Texture quality
    BaseUsage += static_cast<int32>(Settings.TextureQuality) * 256;

    // Ray tracing
    if (Settings.bRayTracingEnabled)
    {
        BaseUsage += 512;
    }

    // Shadow quality
    BaseUsage += static_cast<int32>(Settings.ShadowQuality) * 128;

    return BaseUsage;
}

// ===== PRESETS =====

FS_GraphicsSettings FGraphicsSettingsHandler::GetPreset(EE_GraphicsQuality Quality)
{
    Initialize();

    FS_GraphicsSettings* Preset = GraphicsPresets.Find(Quality);
    if (Preset)
    {
        return *Preset;
    }

    UE_LOG(LogTemp, Warning, TEXT("GraphicsSettingsHandler: Preset not found, returning Medium"));
    return GraphicsPresets[EE_GraphicsQuality::Medium];
}

TArray<EE_GraphicsQuality> FGraphicsSettingsHandler::GetAvailablePresets()
{
    Initialize();

    TArray<EE_GraphicsQuality> Presets;
    GraphicsPresets.GetKeys(Presets);
    return Presets;
}

void FGraphicsSettingsHandler::SetCustomPreset(const FS_GraphicsSettings& Settings)
{
    CustomPreset = Settings;
    CustomPreset.QualityPreset = EE_GraphicsQuality::Custom;
    GraphicsPresets.Add(EE_GraphicsQuality::Custom, CustomPreset);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Custom preset updated"));
}

FS_GraphicsSettings FGraphicsSettingsHandler::GetCustomPreset()
{
    return CustomPreset;
}

// ===== HARDWARE INFO =====

TArray<FIntPoint> FGraphicsSettingsHandler::GetAvailableResolutions()
{
    TArray<FIntPoint> Resolutions;

    // Get screen resolutions from engine
    FScreenResolutionArray ScreenResolutions;
    if (RHIGetAvailableResolutions(ScreenResolutions, true))
    {
        for (const FScreenResolutionRHI& Resolution : ScreenResolutions)
        {
            Resolutions.Add(FIntPoint(Resolution.Width, Resolution.Height));
        }
    }

    // If no resolutions found, add some common ones
    if (Resolutions.Num() == 0)
    {
        Resolutions.Add(FIntPoint(1920, 1080));
        Resolutions.Add(FIntPoint(2560, 1440));
        Resolutions.Add(FIntPoint(3840, 2160));
    }

    return Resolutions;
}

bool FGraphicsSettingsHandler::IsRayTracingSupported()
{
    return GRHISupportsRayTracing;
}

int32 FGraphicsSettingsHandler::GetAvailableVRAM()
{
    // Get VRAM from RHI
    if (GDynamicRHI)
    {
        // Try to get from RHI stats
        FTextureMemoryStats Stats;
        RHIGetTextureMemoryStats(Stats);

        if (Stats.TotalGraphicsMemory > 0)
        {
            return Stats.TotalGraphicsMemory / (1024 * 1024); // Convert to MB
        }
    }

    return 2048; // 2GB default fallback
}

FString FGraphicsSettingsHandler::GetGPUName()
{
    return GRHIAdapterName;
}

int32 FGraphicsSettingsHandler::CalculateGPUPerformanceScore()
{
    int32 Score = 50; // Base score

    int32 VRAMSizeMB = GetAvailableVRAM();
    bool bSupportsRayTracing = IsRayTracingSupported();
    FString GPUName = GetGPUName();

    // VRAM scoring
    if (VRAMSizeMB >= 8192) Score += 20;
    else if (VRAMSizeMB >= 4096) Score += 10;
    else if (VRAMSizeMB >= 2048) Score += 5;

    // Ray tracing support
    if (bSupportsRayTracing) Score += 15;

    // Vendor-specific heuristics
    FString GPUVendor = GRHIVendorId == 0x10DE ? TEXT("NVIDIA") :
        GRHIVendorId == 0x1002 ? TEXT("AMD") :
        GRHIVendorId == 0x8086 ? TEXT("Intel") : TEXT("Unknown");

    if (GPUVendor == TEXT("NVIDIA"))
    {
        if (GPUName.Contains(TEXT("RTX 40"))) Score = 95;
        else if (GPUName.Contains(TEXT("RTX 30"))) Score = 80;
        else if (GPUName.Contains(TEXT("RTX 20"))) Score = 70;
        else if (GPUName.Contains(TEXT("GTX 16"))) Score = 60;
    }
    else if (GPUVendor == TEXT("AMD"))
    {
        if (GPUName.Contains(TEXT("RX 7"))) Score = 85;
        else if (GPUName.Contains(TEXT("RX 6"))) Score = 75;
        else if (GPUName.Contains(TEXT("RX 5"))) Score = 65;
    }

    return FMath::Clamp(Score, 0, 100);
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

// ===== QUALITY SETTINGS =====

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

// ===== BENCHMARKING =====

void FGraphicsSettingsHandler::BenchmarkSettings(float Duration, UWorld* World,
    TFunction<void(EE_GraphicsQuality, float)> OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Starting benchmark (%.1fs)"), Duration);

    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GraphicsSettingsHandler: World is null, cannot benchmark"));
        return;
    }

    // Test different quality levels
    TArray<EE_GraphicsQuality> QualitiesToTest = {
        EE_GraphicsQuality::Low,
        EE_GraphicsQuality::Medium,
        EE_GraphicsQuality::High,
        EE_GraphicsQuality::Ultra
    };

    TArray<float> AverageFPS;

    for (EE_GraphicsQuality Quality : QualitiesToTest)
    {
        // Apply quality preset
        FS_GraphicsSettings TestSettings = GetPreset(Quality);
        ApplyToEngine(TestSettings, World);

        // Wait for settings to apply
        FPlatformProcess::Sleep(1.0f);

        // Measure performance
        float TotalFPS = 0.0f;
        int32 SampleCount = 0;
        float StartTime = UGameplayStatics::GetRealTimeSeconds(World);

        uint64 StartFrame = GFrameNumber; // global frame counter
        double WindowStartTime = FPlatformTime::Seconds();
        const double WindowDuration = Duration / QualitiesToTest.Num();

        while (FPlatformTime::Seconds() - WindowStartTime < WindowDuration)
        {
            const uint64 NowFrame = GFrameNumber;
            const uint64 FramesElapsed = (NowFrame - StartFrame);
            if (FramesElapsed > 0)
            {
                double NowTime = FPlatformTime::Seconds();
                double Elapsed = NowTime - WindowStartTime;
                float InstantFPS = static_cast<float>(FramesElapsed / Elapsed);
                TotalFPS += InstantFPS;
                SampleCount++;
            }
            FPlatformProcess::Sleep(0.1f);
        }

        float AvgFPS = SampleCount > 0 ? TotalFPS / SampleCount : 0.0f;
        AverageFPS.Add(AvgFPS);

        UE_LOG(LogTemp, Log, TEXT("  - %s: %.1f FPS"),
            *UEnum::GetValueAsString(Quality), AvgFPS);

        if (OnComplete)
        {
            OnComplete(Quality, AvgFPS);
        }
    }

    // Determine recommended quality
    EE_GraphicsQuality Recommended = RecommendQualityFromBenchmark(AverageFPS);

    UE_LOG(LogTemp, Log, TEXT("GraphicsSettingsHandler: Benchmark complete - Recommended: %s"),
        *UEnum::GetValueAsString(Recommended));
}

EE_GraphicsQuality FGraphicsSettingsHandler::RecommendQualityFromBenchmark(const TArray<float>& FPSResults)
{
    // Target 60 FPS
    const float TargetFPS = 60.0f;

    // Map: Low=0, Medium=1, High=2, Ultra=3
    for (int32 i = FPSResults.Num() - 1; i >= 0; i--)
    {
        if (FPSResults[i] >= TargetFPS)
        {
            return static_cast<EE_GraphicsQuality>(i);
        }
    }

    // If nothing meets target, return Low
    return EE_GraphicsQuality::Low;
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