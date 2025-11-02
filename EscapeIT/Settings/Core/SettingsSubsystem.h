#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EscapeIT/Data/SettingsStructs.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "SettingsSubsystem.generated.h"

// Forward Declarations
class USettingsSaveManager;

// ===== ENUMS =====
UENUM(BlueprintType)
enum class ESettingsApplyResult : uint8
{
    Success,
    ValidationFailed,
    SaveFailed,
    EngineFailed,
    Cancelled,
    PartialSuccess
};

// ===== DELEGATES =====
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGraphicsSettingsChanged, const FS_GraphicsSettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioSettingsChanged, const FS_AudioSettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplaySettingsChanged, const FS_GameplaySettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnControlSettingsChanged, const FS_ControlSettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccessibilitySettingsChanged, const FS_AccessibilitySettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllSettingsChanged, const FS_AllSettings&, NewSettings);

// Progress callback for async operations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsApplyFailed,const FString&,NewApply);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsConfirmationRequired,float,NewConfirm);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsConfirmed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsReverted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSettingsApplyProgress, ESettingsCategory, Category, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSettingsApplyComplete, ESettingsApplyResult, Result, const FString&, ErrorMessage);

// Hardware info delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHardwareDetected, const FS_HardwareInfo&, HardwareInfo);

UCLASS()
class ESCAPEIT_API USettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ===== LIFECYCLE =====
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ===== DELEGATES =====
    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnGraphicsSettingsChanged OnGraphicsSettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnAudioSettingsChanged OnAudioSettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnGameplaySettingsChanged OnGameplaySettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnControlSettingsChanged OnControlSettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnAccessibilitySettingsChanged OnAccessibilitySettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnAllSettingsChanged OnAllSettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnSettingsApplyProgress OnSettingsApplyProgress;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnSettingsApplyFailed OnSettingsApplyFailed;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnSettingsApplyComplete OnSettingsApplyComplete;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnSettingsConfirmationRequired OnSettingsConfirmationRequired;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnSettingsConfirmed OnSettingsConfirmed;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnSettingsReverted OnSettingsReverted;

    UPROPERTY(BlueprintAssignable, Category = "Settings|Events")
    FOnHardwareDetected OnHardwareDetected;

    // ===== APPLY SETTINGS (IMPROVED) =====

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool ApplyAllSettings(const FS_AllSettings& NewSettings, bool bAsync = false, bool bValidateOnly = false);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool ApplySettingsSynchronously(const FS_AllSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool ApplySettingsCategory(ESettingsCategory Category, bool bAsync = false);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void CancelAsyncApply();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool RollbackSettings();

    UFUNCTION(BlueprintPure, Category = "Settings")
    bool HasUnsavedChanges() const { return bDirtyFlag; }

    UFUNCTION(BlueprintPure, Category = "Settings")
    static bool AreSettingsEqual(const FS_AllSettings& A, const FS_AllSettings& B);

    // ======= APPLY SETTINGS ========= //
    bool ApplyGraphicsSettings(const FS_GraphicsSettings& Settings);
    bool ApplyAudioSettings(const FS_AudioSettings& Settings);
    bool ApplyGameplaySettings(const FS_GameplaySettings& Settings);
    bool ApplyControlSettings(const FS_ControlSettings& Settings);
    bool ApplyAccessibilitySettings(const FS_AccessibilitySettings& Settings);

    // ===== VALIDATION =====

    UFUNCTION(BlueprintCallable, Category = "Settings|Validation")
    bool ValidateSettings(const FS_AllSettings& Settings, TArray<FString>& OutErrors, TArray<FString>& OutWarnings) const;

    UFUNCTION(BlueprintCallable, Category = "Settings|Hardware")
    FS_AllSettings GetRecommendedSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Settings|Hardware")
    FS_GraphicsSettings AutoDetectGraphicsSettings();

    // ===== HARDWARE DETECTION =====

    UFUNCTION(BlueprintCallable, Category = "Settings|Hardware")
    void DetectHardwareCapabilities();

    UFUNCTION(BlueprintPure, Category = "Settings|Hardware")
    FS_HardwareInfo GetHardwareInfo() const { return HardwareInfo; }

    UFUNCTION(BlueprintPure, Category = "Settings|Hardware")
    bool IsRayTracingSupported() const;

    UFUNCTION(BlueprintCallable, Category = "Settings|Hardware")
    TArray<FIntPoint> GetAvailableResolutions() const;

    // ===== SAVE/LOAD (IMPROVED) =====

    UFUNCTION(BlueprintCallable, Category = "Settings|Persistence")
    bool SaveAllSettings(bool bCreateBackup = true);

    UFUNCTION(BlueprintCallable, Category = "Settings|Persistence")
    bool LoadSettings(bool bRestoreBackupOnFailure = true);

    UFUNCTION(BlueprintCallable, Category = "Settings|Persistence")
    void ResetSettingsToDefault(ESettingsCategory Category = ESettingsCategory::All);

    UFUNCTION(BlueprintCallable, Category = "Settings|Persistence")
    bool ExportSettings(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Settings|Persistence")
    bool ImportSettings(const FString& FilePath);

    // ===== PRESETS =====

    UFUNCTION(BlueprintCallable, Category = "Settings|Presets")
    FS_GraphicsSettings GetGraphicsPreset(EE_GraphicsQuality Quality) const;

    UFUNCTION(BlueprintCallable, Category = "Settings|Hardware")
    int32 EstimateVRAMUsage(const FS_GraphicsSettings& Settings) const;

    UFUNCTION(BlueprintPure, Category = "Settings|Presets")
    TArray<EE_GraphicsQuality> GetAvailableGraphicsPresets() const;

    UFUNCTION(BlueprintCallable, Category = "Settings|Presets")
    bool SaveCustomPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Settings|Presets")
    bool LoadCustomPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Settings|Presets")
    bool DeleteCustomPreset(const FString& PresetName);

    UFUNCTION(BlueprintPure, Category = "Settings|Presets")
    TArray<FString> GetCustomPresetNames() const;

    UFUNCTION(BlueprintPure, Category = "Settings|Presets")
    bool SaveCustomPresetsToDisk();

    UFUNCTION(Category = "Settings|Presets")
    void LoadCustomPresetsFromDisk();
    // ===== GETTERS =====

    UFUNCTION(BlueprintPure, Category = "Settings")
    FS_AllSettings GetAllSettings() const { return AllSettings; }

    UFUNCTION(BlueprintPure, Category = "Settings|Graphics")
    FS_GraphicsSettings GetGraphicsSettings() const { return AllSettings.GraphicsSettings; }

    UFUNCTION(BlueprintPure, Category = "Settings|Audio")
    FS_AudioSettings GetAudioSettings() const { return AllSettings.AudioSettings; }

    UFUNCTION(BlueprintPure, Category = "Settings|Gameplay")
    FS_GameplaySettings GetGameplaySettings() const { return AllSettings.GameplaySettings; }

    UFUNCTION(BlueprintPure, Category = "Settings|Controls")
    FS_ControlSettings GetControlSettings() const { return AllSettings.ControlSettings; }

    UFUNCTION(BlueprintPure, Category = "Settings|Accessibility")
    FS_AccessibilitySettings GetAccessibilitySettings() const { return AllSettings.AccessibilitySettings; }

    FS_DifficultyMultiplier GetDifficultyMultiplier(EE_DifficultyLevel DifficultyLevel) const;

    // ===== DIAGNOSTICS =====

    UFUNCTION(BlueprintCallable, Category = "Settings|Debug")
    FString RunDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Settings|Debug")
    void BenchmarkGraphicsSettings(float Duration = 5.0f);

    UFUNCTION(BlueprintPure, Category = "Settings|Debug")
    FS_PerformanceMetrics GetPerformanceMetrics() const;

protected:
    // ===== INTERNAL METHODS =====

    void InitGraphicSettings();
    void InitializeDefaults();
    void MigrateOldSettings(FS_AllSettings& Settings);

    // Validation helpers
    bool ValidateGraphicsSettings(const FS_GraphicsSettings& Settings, TArray<FString>& Errors, TArray<FString>& Warnings) const;
    bool ValidateAudioSettings(const FS_AudioSettings& Settings, TArray<FString>& Errors, TArray<FString>& Warnings) const;
    bool ValidateGameplaySettings(const FS_GameplaySettings& Settings, TArray<FString>& Errors, TArray<FString>& Warnings) const;
    bool ValidateControlSettings(const FS_ControlSettings& Settings, TArray<FString>& Errors, TArray<FString>& Warnings) const;
    bool ValidateAccessibilitySettings(const FS_AccessibilitySettings& Settings, TArray<FString>& Errors, TArray<FString>& Warnings) const;

    // Apply helpers
    bool ApplyGraphicsSettingsToEngine(const FS_GraphicsSettings& Settings);
    bool ApplyAudioSettingsToEngine(const FS_AudioSettings& Settings);
    bool ApplyGameplaySettingsToEngine(const FS_GameplaySettings& Settings);
    bool ApplyControlSettingsToEngine(const FS_ControlSettings& Settings);
    bool ApplyAccessibilitySettingsToEngine(const FS_AccessibilitySettings& Settings);

    // Async apply helpers
    void AsyncApplySettings(const FS_AllSettings& Settings);
    void OnAsyncApplyComplete(ESettingsApplyResult Result, const FString& ErrorMessage);

    // Hardware detection
    void DetectGPU();
    void DetectCPU();
    void DetectRAM();
    void DetectDisplays();

    // Backup/Restore
    bool CreateSettingsBackup();
    bool RestoreSettingsBackup();
    void CleanOldBackups();

private:
    // ===== PROPERTIES =====

    UPROPERTY()
    FS_AllSettings AllSettings;

    UPROPERTY()
    FS_AllSettings PreviousSettings; // For rollback

    UPROPERTY()
    FS_AllSettings BackupSettings; // For backup/restore

    UPROPERTY()
    FS_HardwareInfo HardwareInfo;

    UPROPERTY()
    TMap<EE_GraphicsQuality, FS_GraphicsSettings> GraphicsPresets;

    UPROPERTY()
    TMap<FString, FS_AllSettings> CustomPresets;

    // State flags
    bool bDirtyFlag = false;
    bool bIsApplyingSettings = false;
    bool bCancelAsyncApply = false;
    bool bHardwareDetected = false;

    // Settings versioning
    int32 SettingsVersion = 1;
    const int32 CurrentSettingsVersion = 2;

    // Save slot info
    FString SaveSlotName = TEXT("GameSettings");
    int32 SaveUserIndex = 0;
    FString BackupSlotName = TEXT("GameSettings_Backup");

    // Performance tracking
    mutable FS_PerformanceMetrics CachedMetrics;
    mutable float LastMetricsCacheTime = 0.0f;
    const float MetricsCacheDuration = 1.0f; // Cache for 1 second
    float GAverageFPS;

    // Thread safety
    mutable FCriticalSection SettingsMutex;
};