#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/SettingsTypes.h"
#include "Math/UnrealMathUtility.h"
#include "SettingsStructs.generated.h"

// ===== HARDWARE INFO =====

USTRUCT(BlueprintType)
struct FS_GPUInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FString Vendor;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 VRAMSizeMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    bool bSupportsRayTracing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    bool bSupportsDXR = false;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FString DriverVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 PerformanceScore = 0; // 0-100
};

USTRUCT(BlueprintType)
struct FS_CPUInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FString Vendor;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 CoreCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 ThreadCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    float ClockSpeedGHz = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 PerformanceScore = 0; // 0-100
};

USTRUCT(BlueprintType)
struct FS_MemoryInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 TotalPhysicalGB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 AvailablePhysicalGB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 TotalVirtualGB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 AvailableVirtualGB = 0;
};

USTRUCT(BlueprintType)
struct FS_DisplayInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FIntPoint Resolution;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 RefreshRate = 60;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    bool bIsPrimaryDisplay = false;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FString DisplayName;
};

USTRUCT(BlueprintType)
struct FS_HardwareInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FS_GPUInfo GPU;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FS_CPUInfo CPU;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FS_MemoryInfo Memory;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    TArray<FS_DisplayInfo> Displays;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FString OperatingSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    int32 OverallPerformanceScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Hardware")
    FDateTime DetectionTime;
};

// ===== PERFORMANCE METRICS =====

USTRUCT(BlueprintType)
struct FS_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f; // in ms

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f; // in ms

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime = 0.0f; // in ms

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedVRAMMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedRAMMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FDateTime MeasurementTime;
};

// ===== VALIDATION RESULT =====

USTRUCT(BlueprintType)
struct FS_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = true;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Suggestions;
};

// ===== SETTINGS METADATA =====

USTRUCT(BlueprintType)
struct FS_SettingsMetadata
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metadata")
    int32 Version = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Metadata")
    FDateTime LastModified;

    UPROPERTY(BlueprintReadOnly, Category = "Metadata")
    FDateTime CreationDate;

    UPROPERTY(BlueprintReadOnly, Category = "Metadata")
    FString PresetName;

    UPROPERTY(BlueprintReadOnly, Category = "Metadata")
    bool bIsCustomPreset = false;

    UPROPERTY(BlueprintReadOnly, Category = "Metadata")
    FS_HardwareInfo HardwareSnapshot;
};

// ===== GRAPHICS CAPABILITIES =====

USTRUCT(BlueprintType)
struct FS_GraphicsCapabilities
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Capabilities")
    TArray<FIntPoint> SupportedResolutions;

    UPROPERTY(BlueprintReadOnly, Category = "Capabilities")
    int32 MaxTextureSize = 4096;

    UPROPERTY(BlueprintReadOnly, Category = "Capabilities")
    bool bSupportsRayTracing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Capabilities")
    bool bSupportsMeshShaders = false;

    UPROPERTY(BlueprintReadOnly, Category = "Capabilities")
    bool bSupportsVariableRateShading = false;

    UPROPERTY(BlueprintReadOnly, Category = "Capabilities")
    int32 MaxAnisotropy = 16;

    UPROPERTY(BlueprintReadOnly, Category = "Capabilities")
    TArray<FString> SupportedAntiAliasingMethods;

    UPROPERTY(BlueprintReadOnly, Category = "Capabilities")
    int32 RecommendedVRAMUsageMB = 2048;
};

// ===== BENCHMARK RESULT =====

USTRUCT(BlueprintType)
struct FS_BenchmarkResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    EE_GraphicsQuality RecommendedPreset;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    FS_PerformanceMetrics Metrics;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float BenchmarkScore = 0.0f; // 0-100

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float DurationSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    FDateTime BenchmarkTime;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    TArray<FString> PerformanceNotes;
};

// ===== SETTINGS CHANGE EVENT =====

USTRUCT(BlueprintType)
struct FS_SettingsChangeEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Event")
    ESettingsCategory Category;

    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FString PropertyName;

    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FString OldValue;

    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FString NewValue;

    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FDateTime Timestamp;
};

// ===== QUALITY RECOMMENDATION =====

USTRUCT(BlueprintType)
struct FS_QualityRecommendation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Recommendation")
    EE_GraphicsQuality RecommendedPreset;

    UPROPERTY(BlueprintReadOnly, Category = "Recommendation")
    FS_GraphicsSettings RecommendedSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Recommendation")
    float ConfidenceScore = 0.0f; // 0-1

    UPROPERTY(BlueprintReadOnly, Category = "Recommendation")
    TArray<FString> Reasoning;

    UPROPERTY(BlueprintReadOnly, Category = "Recommendation")
    bool bRequiresBenchmark = false;
};

// -----------------------------------------------------------------------------
//  Non-member equality operators for settings structs
//  NOTE: these operators expect that FS_GraphicsSettings and FS_AudioSettings
//  are declared in one of the included headers (SettingsTypes.h).
// -----------------------------------------------------------------------------

// Tolerance used for float comparisons
static constexpr float SETTINGS_FLOAT_COMPARE_TOLERANCE = 1e-6f;

inline bool operator==(const FS_GraphicsSettings& A, const FS_GraphicsSettings& B) noexcept
{
    return A.QualityPreset == B.QualityPreset &&
        A.ResolutionX == B.ResolutionX &&
        A.ResolutionY == B.ResolutionY &&
        A.bVSyncEnabled == B.bVSyncEnabled &&
        A.FrameRateCap == B.FrameRateCap &&
        A.bRayTracingEnabled == B.bRayTracingEnabled &&
        A.RayTracingQuality == B.RayTracingQuality &&
        A.ShadowQuality == B.ShadowQuality &&
        A.TextureQuality == B.TextureQuality &&
        A.AntiAliasingMethod == B.AntiAliasingMethod &&
        FMath::IsNearlyEqual(A.MotionBlurAmount, B.MotionBlurAmount, SETTINGS_FLOAT_COMPARE_TOLERANCE) &&
        FMath::IsNearlyEqual(A.FieldOfView, B.FieldOfView, SETTINGS_FLOAT_COMPARE_TOLERANCE);
}

inline bool operator!=(const FS_GraphicsSettings& A, const FS_GraphicsSettings& B) noexcept
{
    //return !(A == B);
}

inline bool operator==(const FS_AudioSettings& A, const FS_AudioSettings& B) noexcept
{
    return FMath::IsNearlyEqual(A.MasterVolume, B.MasterVolume, SETTINGS_FLOAT_COMPARE_TOLERANCE) &&
        FMath::IsNearlyEqual(A.MusicVolume, B.MusicVolume, SETTINGS_FLOAT_COMPARE_TOLERANCE) &&
        FMath::IsNearlyEqual(A.SFXVolume, B.SFXVolume, SETTINGS_FLOAT_COMPARE_TOLERANCE) &&
        FMath::IsNearlyEqual(A.AmbientVolume, B.AmbientVolume, SETTINGS_FLOAT_COMPARE_TOLERANCE) &&
        FMath::IsNearlyEqual(A.UIVolume, B.UIVolume, SETTINGS_FLOAT_COMPARE_TOLERANCE) &&
        FMath::IsNearlyEqual(A.DialogueVolume, B.DialogueVolume, SETTINGS_FLOAT_COMPARE_TOLERANCE) &&
        A.CurrentLanguage == B.CurrentLanguage &&
        A.AudioOutput == B.AudioOutput &&
        A.bClosedCaptionsEnabled == B.bClosedCaptionsEnabled &&
        A.bSubtitlesEnabled == B.bSubtitlesEnabled;
}

inline bool operator!=(const FS_AudioSettings& A, const FS_AudioSettings& B) noexcept
{
    //return !(A == B);
}