#pragma once

#include "CoreMinimal.h"
#include "Data/SettingsStructs.h"

class ESCAPEIT_API FGraphicsSettingsHandler
{
public:
    // ===== INITIALIZATION =====
    static void Initialize();
    static void InitializePresets();

    // ===== MAIN APPLY FUNCTION =====
    static void ApplyToEngine(const FS_GraphicsSettings& Settings, UWorld* World);

    // ===== AUTO-DETECT =====
    static FS_GraphicsSettings AutoDetectSettings();

    // ===== VALIDATION =====
    static bool ValidateSettings(const FS_GraphicsSettings& Settings, TArray<FString>& OutErrors, TArray<FString>& OutWarnings);
    static int32 EstimateVRAMUsage(const FS_GraphicsSettings& Settings);

    // ===== PRESETS =====
    static FS_GraphicsSettings GetPreset(EE_GraphicsQuality Quality);
    static TArray<EE_GraphicsQuality> GetAvailablePresets();
    static void SetCustomPreset(const FS_GraphicsSettings& Settings);
    static FS_GraphicsSettings GetCustomPreset();

    // ===== HARDWARE INFO =====
    static TArray<FIntPoint> GetAvailableResolutions();
    static bool IsRayTracingSupported();
    static int32 GetAvailableVRAM();
    static FString GetGPUName();
    static int32 CalculateGPUPerformanceScore();

    // ===== INDIVIDUAL SETTERS =====
    static void SetResolution(int32 X, int32 Y);
    static void SetVSync(bool bEnabled);
    static void SetFrameRateLimit(int32 FrameRate);
    static void SetOverallQuality(EE_GraphicsQuality Quality);
    static void SetRayTracing(bool bEnabled, EE_RayTracingQuality Quality, UWorld* World);
    static void SetMotionBlur(float Amount, UWorld* World);
    static void SetFieldOfView(float FOV, UWorld* World);

    // ===== QUALITY SETTINGS =====
    static void SetViewDistance(int32 Quality);
    static void SetShadowQuality(int32 Quality);
    static void SetTextureQuality(int32 Quality);
    static void SetAntiAliasing(int32 Quality);
    static void SetPostProcessing(int32 Quality);
    static void SetEffectsQuality(int32 Quality);
    static void SetFoliageQuality(int32 Quality);
    static void SetShadingQuality(int32 Quality);

    // ===== BENCHMARKING =====
    static void BenchmarkSettings(float Duration, UWorld* World, TFunction<void(EE_GraphicsQuality, float)> OnComplete);
    static EE_GraphicsQuality RecommendQualityFromBenchmark(const TArray<float>& FPSResults);

private:
    // ===== HELPER FUNCTIONS =====
    static class UGameUserSettings* GetGameUserSettings();
    static void ExecuteConsoleCommand(UWorld* World, const FString& Command);

    // ===== INTERNAL DATA =====
    static TMap<EE_GraphicsQuality, FS_GraphicsSettings> GraphicsPresets;
    static FS_GraphicsSettings CustomPreset;
    static bool bPresetsInitialized;
};