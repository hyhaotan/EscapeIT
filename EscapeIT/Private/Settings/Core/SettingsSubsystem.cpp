
#include "Settings/Core/SettingsSubsystem.h"
#include "Settings/Core/SettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "RHI.h"
#include "DynamicRHI.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "SaveGames/SettingsSaveManager.h"
#include "HAL/PlatformMemory.h"
#include "Async/Async.h"
#include "RendererInterface.h"
#include "Settings/Validators/SettingsValidator.h"
#include "Settings/Handlers/GraphicsSettingsHandler.h"
#include "Settings/Handlers/AudioSettingsHandler.h"
#include "Settings/Handlers/GameplaySettingsHandler.h"
#include "Settings/Handlers/AccessibilitySettingsHandler.h"
#include "Settings/Handlers/ControlSettingsHandler.h"

void USettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Initializing..."));

    // Initialize handlers
    FGraphicsSettingsHandler::Initialize();

    // Initialize defaults
    InitializeDefaults();

    // Detect hardware capabilities
    DetectHardwareCapabilities();

    // Load saved settings
    LoadSettings(true);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Initialized successfully"));
}

void USettingsSubsystem::Deinitialize()
{
    // Auto-save on shutdown if there are unsaved changes
    if (bDirtyFlag)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Unsaved changes detected, auto-saving..."));
        SaveAllSettings(false);
    }

    Super::Deinitialize();
}

// ===== IMPROVED APPLY SETTINGS =====

bool USettingsSubsystem::ApplyAllSettings(const FS_AllSettings& NewSettings, bool bAsync, bool bValidateOnly)
{
    FScopeLock Lock(&SettingsMutex);

    // Prevent concurrent apply operations
    if (bIsApplyingSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Settings application already in progress"));
        return false;
    }

    // Validate settings first
    TArray<FString> Errors, Warnings;
    if (!ValidateSettings(NewSettings, Errors, Warnings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Validation failed with %d errors"), Errors.Num());
        for (const FString& Error : Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }

        OnSettingsApplyComplete.Broadcast(ESettingsApplyResult::ValidationFailed,
            FString::Printf(TEXT("Validation failed: %d errors"), Errors.Num()));
        return false;
    }

    // Log warnings
    for (const FString& Warning : Warnings)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: %s"), *Warning);
    }

    // If validation only, return here
    if (bValidateOnly)
    {
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Validation passed (validation only mode)"));
        return true;
    }

    // Store previous settings for rollback
    PreviousSettings = AllSettings;

    // Apply synchronously or asynchronously
    if (bAsync)
    {
        bIsApplyingSettings = true;
        AsyncApplySettings(NewSettings);
        return true;
    }
    else
    {
        return ApplySettingsSynchronously(NewSettings);
    }
}

bool USettingsSubsystem::ApplySettingsSynchronously(const FS_AllSettings& NewSettings)
{
    bool bSuccess = true;
    ESettingsApplyResult Result = ESettingsApplyResult::Success;
    FString ErrorMessage;

    // Update settings
    AllSettings = NewSettings;

    // Apply each category
    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Graphics, 0.0f);
    if (!ApplyGraphicsSettingsToEngine(AllSettings.GraphicsSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to apply graphics settings"));
        bSuccess = false;
        ErrorMessage += TEXT("Graphics settings failed. ");
    }
    OnGraphicsSettingsChanged.Broadcast(AllSettings.GraphicsSettings);
    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Graphics, 1.0f);

    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Audio, 0.0f);
    if (!ApplyAudioSettingsToEngine(AllSettings.AudioSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to apply audio settings"));
        bSuccess = false;
        ErrorMessage += TEXT("Audio settings failed. ");
    }
    OnAudioSettingsChanged.Broadcast(AllSettings.AudioSettings);
    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Audio, 1.0f);

    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Gameplay, 0.0f);
    if (!ApplyGameplaySettingsToEngine(AllSettings.GameplaySettings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to apply gameplay settings"));
        bSuccess = false;
        ErrorMessage += TEXT("Gameplay settings failed. ");
    }
    OnGameplaySettingsChanged.Broadcast(AllSettings.GameplaySettings);
    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Gameplay, 1.0f);

    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Controls, 0.0f);
    if (!ApplyControlSettingsToEngine(AllSettings.ControlSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to apply control settings"));
        bSuccess = false;
        ErrorMessage += TEXT("Control settings failed. ");
    }
    OnControlSettingsChanged.Broadcast(AllSettings.ControlSettings);
    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Controls, 1.0f);

    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Accessibility, 0.0f);
    if (!ApplyAccessibilitySettingsToEngine(AllSettings.AccessibilitySettings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to apply accessibility settings"));
        bSuccess = false;
        ErrorMessage += TEXT("Accessibility settings failed. ");
    }
    OnAccessibilitySettingsChanged.Broadcast(AllSettings.AccessibilitySettings);
    OnSettingsApplyProgress.Broadcast(ESettingsCategory::Accessibility, 1.0f);

    // Broadcast all settings changed
    OnAllSettingsChanged.Broadcast(AllSettings);

    // Save to disk
    if (!SaveAllSettings(true))
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Settings applied but failed to save"));
        Result = ESettingsApplyResult::PartialSuccess;
        ErrorMessage += TEXT("Failed to save settings. ");
    }

    // Set dirty flag
    bDirtyFlag = false;

    // Determine final result
    if (!bSuccess)
    {
        Result = ESettingsApplyResult::EngineFailed;

        // Rollback on failure
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Rolling back due to errors"));
        RollbackSettings();
    }

    OnSettingsApplyComplete.Broadcast(Result, ErrorMessage);

    return bSuccess;
}

void USettingsSubsystem::AsyncApplySettings(const FS_AllSettings& Settings)
{
    // Run on async thread
    Async(EAsyncExecution::ThreadPool, [this, Settings]()
        {
            // Check if cancelled
            if (bCancelAsyncApply)
            {
                bCancelAsyncApply = false;
                bIsApplyingSettings = false;

                AsyncTask(ENamedThreads::GameThread, [this]()
                    {
                        OnSettingsApplyComplete.Broadcast(ESettingsApplyResult::Cancelled, TEXT("Operation cancelled by user"));
                    });
                return;
            }

            // Apply settings on game thread
            AsyncTask(ENamedThreads::GameThread, [this, Settings]()
                {
                    bool bSuccess = ApplySettingsSynchronously(Settings);
                    bIsApplyingSettings = false;
                });
        });
}

void USettingsSubsystem::OnAsyncApplyComplete(ESettingsApplyResult Result, const FString& ErrorMessage)
{
}

void USettingsSubsystem::CancelAsyncApply()
{
    if (bIsApplyingSettings)
    {
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Cancelling async apply operation"));
        bCancelAsyncApply = true;
    }
}

bool USettingsSubsystem::RollbackSettings()
{
    FScopeLock Lock(&SettingsMutex);

    if (PreviousSettings.GraphicsSettings.ResolutionX == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: No previous settings to rollback to"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Rolling back to previous settings"));

    // Apply previous settings
    bool bSuccess = ApplySettingsSynchronously(PreviousSettings);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Rollback successful"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Rollback failed!"));
    }

    return bSuccess;
}

// ===== IMPROVED VALIDATION =====

bool USettingsSubsystem::ValidateSettings(const FS_AllSettings& Settings,
    TArray<FString>& OutErrors, TArray<FString>& OutWarnings) const
{
    OutErrors.Empty();
    OutWarnings.Empty();

    bool bValid = true;

    // Validate each category
    bValid &= ValidateGraphicsSettings(Settings.GraphicsSettings, OutErrors, OutWarnings);
    bValid &= ValidateAudioSettings(Settings.AudioSettings, OutErrors, OutWarnings);
    bValid &= ValidateGameplaySettings(Settings.GameplaySettings, OutErrors, OutWarnings);
    bValid &= ValidateControlSettings(Settings.ControlSettings, OutErrors, OutWarnings);
    bValid &= ValidateAccessibilitySettings(Settings.AccessibilitySettings, OutErrors, OutWarnings);

    return bValid;
}

bool USettingsSubsystem::ValidateGraphicsSettings(const FS_GraphicsSettings& Settings,
    TArray<FString>& Errors, TArray<FString>& Warnings) const
{
    FString ErrorMsg;

    // Sử dụng FSettingsValidator trước
    if (!FSettingsValidator::ValidateGraphicsSettings(Settings, ErrorMsg))
    {
        Errors.Add(ErrorMsg);
        return false;
    }

    bool bValid = true;

    // Thêm các validation đặc biệt (không có trong FSettingsValidator)

    // Check if resolution is supported
    TArray<FIntPoint> SupportedResolutions = GetAvailableResolutions();
    FIntPoint RequestedRes(Settings.ResolutionX, Settings.ResolutionY);
    if (!SupportedResolutions.Contains(RequestedRes))
    {
        Warnings.Add(FString::Printf(TEXT("Resolution %dx%d may not be supported by your display"),
            Settings.ResolutionX, Settings.ResolutionY));
    }

    // Ray tracing validation
    if (Settings.bRayTracingEnabled && !IsRayTracingSupported())
    {
        Errors.Add(TEXT("Ray tracing is not supported on this hardware"));
        bValid = false;
    }

    // VRAM check
    if (bHardwareDetected)
    {
        int32 EstimatedVRAMUsage = EstimateVRAMUsage(Settings);
        if (EstimatedVRAMUsage > HardwareInfo.GPU.VRAMSizeMB)
        {
            Warnings.Add(FString::Printf(
                TEXT("Settings may exceed available VRAM (%d MB required, %d MB available)"),
                EstimatedVRAMUsage, HardwareInfo.GPU.VRAMSizeMB));
        }
    }

    return bValid;
}

bool USettingsSubsystem::ValidateAudioSettings(const FS_AudioSettings& Settings,
    TArray<FString>& Errors, TArray<FString>& Warnings) const
{
    FString ErrorMsg;
    bool bValid = FSettingsValidator::ValidateAudioSettings(Settings, ErrorMsg);

    if (!bValid)
    {
        Errors.Add(ErrorMsg);
    }

    // Thêm warnings nếu cần
    if (Settings.MasterVolume == 0.0f)
    {
        Warnings.Add(TEXT("Master volume is muted"));
    }

    return bValid;
}

bool USettingsSubsystem::ValidateGameplaySettings(const FS_GameplaySettings& Settings,
    TArray<FString>& Errors, TArray<FString>& Warnings) const
{
    FString ErrorMsg;
    bool bValid = FSettingsValidator::ValidateGameplaySettings(Settings, ErrorMsg);

    if (!bValid)
    {
        Errors.Add(ErrorMsg);
    }

    // Thêm warnings
    if (Settings.MouseSensitivity < 0.3f)
    {
        Warnings.Add(TEXT("Mouse sensitivity is very low"));
    }
    else if (Settings.MouseSensitivity > 5.0f)
    {
        Warnings.Add(TEXT("Mouse sensitivity is very high"));
    }

    return bValid;
}

bool USettingsSubsystem::ValidateControlSettings(const FS_ControlSettings& Settings,
    TArray<FString>& Errors, TArray<FString>& Warnings) const
{
    FString ErrorMsg;
    bool bValid = FSettingsValidator::ValidateControlSettings(Settings, ErrorMsg);

    if (!bValid)
    {
        Errors.Add(ErrorMsg);
    }

    // Thêm warnings nếu cần
    if (Settings.GamepadDeadzone > 0.3f)
    {
        Warnings.Add(TEXT("Gamepad deadzone is quite high"));
    }

    return bValid;
}

bool USettingsSubsystem::ValidateAccessibilitySettings(const FS_AccessibilitySettings& Settings,
    TArray<FString>& Errors, TArray<FString>& Warnings) const
{
    FString ErrorMsg;
    bool bValid = FSettingsValidator::ValidateAccessibilitySettings(Settings, ErrorMsg);

    if (!bValid)
    {
        Errors.Add(ErrorMsg);
    }

    // Thêm warnings
    if (Settings.HoldActivationTime > 2.0f)
    {
        Warnings.Add(TEXT("Hold activation time is quite long"));
    }

    return bValid;
}

int32 USettingsSubsystem::EstimateVRAMUsage(const FS_GraphicsSettings& Settings) const
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

// ===== HARDWARE DETECTION =====

void USettingsSubsystem::DetectHardwareCapabilities()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Detecting hardware capabilities..."));

    DetectGPU();
    DetectCPU();
    DetectRAM();
    DetectDisplays();

    // Calculate overall performance score
    HardwareInfo.OverallPerformanceScore =
        (HardwareInfo.GPU.PerformanceScore +
            HardwareInfo.CPU.PerformanceScore) / 2;

    HardwareInfo.DetectionTime = FDateTime::UtcNow();
    bHardwareDetected = true;

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Hardware detection complete"));
    UE_LOG(LogTemp, Log, TEXT("  - GPU: %s (Score: %d)"),
        *HardwareInfo.GPU.Name, HardwareInfo.GPU.PerformanceScore);
    UE_LOG(LogTemp, Log, TEXT("  - CPU: %s (Score: %d)"),
        *HardwareInfo.CPU.Name, HardwareInfo.CPU.PerformanceScore);
    UE_LOG(LogTemp, Log, TEXT("  - RAM: %d GB"), HardwareInfo.Memory.TotalPhysicalGB);
    UE_LOG(LogTemp, Log, TEXT("  - Overall Score: %d"), HardwareInfo.OverallPerformanceScore);

    OnHardwareDetected.Broadcast(HardwareInfo);
}

void USettingsSubsystem::DetectGPU()
{
    // Use handler to get GPU info
    HardwareInfo.GPU.Name = FGraphicsSettingsHandler::GetGPUName();
    HardwareInfo.GPU.VRAMSizeMB = FGraphicsSettingsHandler::GetAvailableVRAM();
    HardwareInfo.GPU.bSupportsRayTracing = FGraphicsSettingsHandler::IsRayTracingSupported();
    HardwareInfo.GPU.bSupportsDXR = HardwareInfo.GPU.bSupportsRayTracing;
    HardwareInfo.GPU.PerformanceScore = FGraphicsSettingsHandler::CalculateGPUPerformanceScore();

    // Determine vendor
    HardwareInfo.GPU.Vendor = GRHIVendorId == 0x10DE ? TEXT("NVIDIA") :
        GRHIVendorId == 0x1002 ? TEXT("AMD") :
        GRHIVendorId == 0x8086 ? TEXT("Intel") : TEXT("Unknown");
}

void USettingsSubsystem::DetectCPU()
{
    // Get CPU info from platform
    HardwareInfo.CPU.Name = FPlatformMisc::GetCPUBrand();
    HardwareInfo.CPU.Vendor = FPlatformMisc::GetCPUVendor();
    HardwareInfo.CPU.CoreCount = FPlatformMisc::NumberOfCores();
    HardwareInfo.CPU.ThreadCount = FPlatformMisc::NumberOfCoresIncludingHyperthreads();

    // Rough performance scoring
    int32 Score = 40;

    if (HardwareInfo.CPU.CoreCount >= 16)
        Score += 30;
    else if (HardwareInfo.CPU.CoreCount >= 8)
        Score += 20;
    else if (HardwareInfo.CPU.CoreCount >= 6)
        Score += 15;
    else if (HardwareInfo.CPU.CoreCount >= 4)
        Score += 10;

    HardwareInfo.CPU.PerformanceScore = FMath::Clamp(Score, 0, 100);
}

void USettingsSubsystem::DetectRAM()
{
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

    HardwareInfo.Memory.TotalPhysicalGB =
        MemStats.TotalPhysical / (1024 * 1024 * 1024);
    HardwareInfo.Memory.AvailablePhysicalGB =
        MemStats.AvailablePhysical / (1024 * 1024 * 1024);
    HardwareInfo.Memory.TotalVirtualGB =
        MemStats.TotalVirtual / (1024 * 1024 * 1024);
    HardwareInfo.Memory.AvailableVirtualGB =
        MemStats.AvailableVirtual / (1024 * 1024 * 1024);
}

void USettingsSubsystem::DetectDisplays()
{
    // Get desktop resolution
    FDisplayMetrics DisplayMetrics;
    FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);

    FS_DisplayInfo PrimaryDisplay;
    PrimaryDisplay.Resolution = FIntPoint(
        DisplayMetrics.PrimaryDisplayWidth,
        DisplayMetrics.PrimaryDisplayHeight);
    PrimaryDisplay.RefreshRate = 60; // Default
    PrimaryDisplay.bIsPrimaryDisplay = true;
    PrimaryDisplay.DisplayName = TEXT("Primary Display");

    HardwareInfo.Displays.Add(PrimaryDisplay);
}

TArray<FIntPoint> USettingsSubsystem::GetAvailableResolutions() const
{
    return FGraphicsSettingsHandler::GetAvailableResolutions();
}

bool USettingsSubsystem::IsRayTracingSupported() const
{
    return FGraphicsSettingsHandler::IsRayTracingSupported();
}

// ===== AUTO-DETECT & RECOMMENDATIONS =====

FS_AllSettings USettingsSubsystem::GetRecommendedSettings() const
{
    FS_AllSettings Recommended;

    if (!bHardwareDetected)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Hardware not detected, using defaults"));
        Recommended.GraphicsSettings = FGraphicsSettingsHandler::GetPreset(EE_GraphicsQuality::Medium);
        return Recommended;
    }

    // Use handler's auto-detect for graphics
    Recommended.GraphicsSettings = FGraphicsSettingsHandler::AutoDetectSettings();

    // Other settings use defaults
    // Audio, Gameplay, Controls, Accessibility remain as default

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Recommended settings generated"));

    return Recommended;
}

FS_GraphicsSettings USettingsSubsystem::AutoDetectGraphicsSettings()
{
    return FGraphicsSettingsHandler::AutoDetectSettings();
}

// ===== COMPARISON =====

bool USettingsSubsystem::AreSettingsEqual(const FS_AllSettings& A, const FS_AllSettings& B)
{
    return A.GameplaySettings == B.GameplaySettings &&
        A.ControlSettings == B.ControlSettings &&
        A.AccessibilitySettings == B.AccessibilitySettings;
}

// ===== IMPROVED SAVE/LOAD =====

bool USettingsSubsystem::SaveAllSettings(bool bCreateBackup)
{
    FScopeLock Lock(&SettingsMutex);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Saving settings to slot '%s'"), *SaveSlotName);

    // Create backup if requested
    if (bCreateBackup)
    {
        if (!CreateSettingsBackup())
        {
            UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Failed to create backup, continuing with save"));
        }
    }

    // Create save game object
    USettingsSaveManager* Save = Cast<USettingsSaveManager>(
        UGameplayStatics::CreateSaveGameObject(USettingsSaveManager::StaticClass())
    );

    if (!Save)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to create save game object"));
        return false;
    }

    // Populate save data
    Save->AllSettings = AllSettings;
    Save->LastSavedTime = FDateTime::UtcNow();
    Save->SettingsVersion = CurrentSettingsVersion;
    Save->HardwareSnapshot = HardwareInfo;

    // Attempt to save
    bool bSaveSuccess = UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, SaveUserIndex);

    if (bSaveSuccess)
    {
        bDirtyFlag = false;
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Settings saved successfully"));

        // Clean old backups (keep only last 3)
        CleanOldBackups();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to save settings"));
    }

    return bSaveSuccess;
}

bool USettingsSubsystem::LoadSettings(bool bRestoreBackupOnFailure)
{
    FScopeLock Lock(&SettingsMutex);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Loading settings from slot '%s'"), *SaveSlotName);

    // Check if save exists
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: No save file found, using defaults"));

        // Try to load from backup
        if (bRestoreBackupOnFailure)
        {
            if (RestoreSettingsBackup())
            {
                return true;
            }
        }

        // Use defaults
        InitializeDefaults();
        return false;
    }

    // Load save game
    USettingsSaveManager* Save = Cast<USettingsSaveManager>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex)
    );

    if (!Save)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to load save game"));

        // Try backup
        if (bRestoreBackupOnFailure)
        {
            if (RestoreSettingsBackup())
            {
                return true;
            }
        }

        InitializeDefaults();
        return false;
    }

    // Check version and migrate if needed
    if (Save->SettingsVersion < CurrentSettingsVersion)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Old settings version detected (%d -> %d), migrating..."),
            Save->SettingsVersion, CurrentSettingsVersion);

        MigrateOldSettings(Save->AllSettings);
        Save->SettingsVersion = CurrentSettingsVersion;

        // Save migrated settings
        UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, SaveUserIndex);
    }

    // Validate loaded settings
    TArray<FString> Errors, Warnings;
    if (!ValidateSettings(Save->AllSettings, Errors, Warnings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Loaded settings failed validation"));

        // Try backup
        if (bRestoreBackupOnFailure)
        {
            if (RestoreSettingsBackup())
            {
                return true;
            }
        }

        // Fall back to defaults
        InitializeDefaults();
        return false;
    }

    // Apply loaded settings
    AllSettings = Save->AllSettings;

    // Apply to engine
    ApplyGraphicsSettingsToEngine(AllSettings.GraphicsSettings);
    ApplyAudioSettingsToEngine(AllSettings.AudioSettings);
    ApplyGameplaySettingsToEngine(AllSettings.GameplaySettings);
    ApplyControlSettingsToEngine(AllSettings.ControlSettings);
    ApplyAccessibilitySettingsToEngine(AllSettings.AccessibilitySettings);

    // Broadcast changes
    OnAllSettingsChanged.Broadcast(AllSettings);

    bDirtyFlag = false;

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Settings loaded successfully (Last saved: %s)"),
        *Save->LastSavedTime.ToString());

    return true;
}

bool USettingsSubsystem::CreateSettingsBackup()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Creating settings backup"));

    // Check if current save exists
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: No existing save to backup"));
        return false;
    }

    // Load current save
    USettingsSaveManager* CurrentSave = Cast<USettingsSaveManager>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex)
    );

    if (!CurrentSave)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to load current save for backup"));
        return false;
    }

    // Generate timestamped backup name
    FString BackupName = FString::Printf(TEXT("%s_%s"),
        *BackupSlotName,
        *FDateTime::UtcNow().ToString(TEXT("%Y%m%d_%H%M%S")));

    // Save backup
    bool bSuccess = UGameplayStatics::SaveGameToSlot(CurrentSave, BackupName, SaveUserIndex);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Backup created: %s"), *BackupName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to create backup"));
    }

    return bSuccess;
}

bool USettingsSubsystem::RestoreSettingsBackup()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Attempting to restore from backup"));

    // Find most recent backup
    TArray<FString> BackupFiles;

    // Search for backup files (simplified - in production you'd scan the save directory)
    for (int i = 0; i < 5; i++)
    {
        FString BackupName = FString::Printf(TEXT("%s_%d"), *BackupSlotName, i);
        if (UGameplayStatics::DoesSaveGameExist(BackupName, SaveUserIndex))
        {
            BackupFiles.Add(BackupName);
        }
    }

    if (BackupFiles.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: No backup files found"));
        return false;
    }

    // Try to load most recent backup
    for (const FString& BackupName : BackupFiles)
    {
        USettingsSaveManager* BackupSave = Cast<USettingsSaveManager>(
            UGameplayStatics::LoadGameFromSlot(BackupName, SaveUserIndex)
        );

        if (BackupSave)
        {
            // Validate backup
            TArray<FString> Errors, Warnings;
            if (ValidateSettings(BackupSave->AllSettings, Errors, Warnings))
            {
                // Restore backup
                AllSettings = BackupSave->AllSettings;

                // Save as main save
                UGameplayStatics::SaveGameToSlot(BackupSave, SaveSlotName, SaveUserIndex);

                UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Restored from backup: %s"), *BackupName);
                return true;
            }
        }
    }

    UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to restore from any backup"));
    return false;
}

void USettingsSubsystem::CleanOldBackups()
{
    // Keep only the 3 most recent backups
    // In production, you'd implement proper directory scanning
    const int32 MaxBackups = 3;

    // This is simplified - you'd need to scan the save directory
    // and sort backups by timestamp, then delete old ones

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Cleaning old backups"));
}

void USettingsSubsystem::MigrateOldSettings(FS_AllSettings& Settings)
{
    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Migrating settings to version %d"), CurrentSettingsVersion);

    // Version 1 -> 2 migration example
    if (SettingsVersion < 2)
    {
        // Add new settings with defaults
        Settings.GameplaySettings.bAllowSkipPuzzles = false;
        Settings.AccessibilitySettings.HoldActivationTime = 0.5f;

        UE_LOG(LogTemp, Log, TEXT("  - Migrated from version 1 to 2"));
    }

    // Future migrations would go here
    // if (SettingsVersion < 3) { ... }
}

void USettingsSubsystem::InitializeDefaults()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Initializing default settings"));

    // Set default settings
    AllSettings = FS_AllSettings();

    // Apply default preset based on hardware
    if (bHardwareDetected)
    {
        FS_AllSettings Recommended = GetRecommendedSettings();
        AllSettings.GraphicsSettings = Recommended.GraphicsSettings;

        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Applied recommended settings based on hardware"));
    }
    else
    {
        AllSettings.GraphicsSettings = FGraphicsSettingsHandler::GetPreset(EE_GraphicsQuality::Medium);
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Applied medium preset as default"));
    }
}

// ===== EXPORT/IMPORT =====

bool USettingsSubsystem::ExportSettings(const FString& FilePath)
{
    FScopeLock Lock(&SettingsMutex);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Exporting settings to %s"), *FilePath);

    // Create JSON object
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    // Version info
    JsonObject->SetNumberField(TEXT("Version"), CurrentSettingsVersion);
    JsonObject->SetStringField(TEXT("ExportDate"), FDateTime::UtcNow().ToString());

    // Graphics settings
    TSharedPtr<FJsonObject> GraphicsJson = MakeShareable(new FJsonObject);
    GraphicsJson->SetNumberField(TEXT("QualityPreset"), static_cast<int32>(AllSettings.GraphicsSettings.QualityPreset));
    GraphicsJson->SetNumberField(TEXT("ResolutionX"), AllSettings.GraphicsSettings.ResolutionX);
    GraphicsJson->SetNumberField(TEXT("ResolutionY"), AllSettings.GraphicsSettings.ResolutionY);
    GraphicsJson->SetBoolField(TEXT("VSyncEnabled"), AllSettings.GraphicsSettings.bVSyncEnabled);
    GraphicsJson->SetNumberField(TEXT("FrameRateCap"), AllSettings.GraphicsSettings.FrameRateCap);
    GraphicsJson->SetBoolField(TEXT("RayTracingEnabled"), AllSettings.GraphicsSettings.bRayTracingEnabled);
    GraphicsJson->SetNumberField(TEXT("FieldOfView"), AllSettings.GraphicsSettings.FieldOfView);
    JsonObject->SetObjectField(TEXT("Graphics"), GraphicsJson);

    // Audio settings
    TSharedPtr<FJsonObject> AudioJson = MakeShareable(new FJsonObject);
    AudioJson->SetNumberField(TEXT("MasterVolume"), AllSettings.AudioSettings.MasterVolume);
    AudioJson->SetNumberField(TEXT("MusicVolume"), AllSettings.AudioSettings.MusicVolume);
    AudioJson->SetNumberField(TEXT("SFXVolume"), AllSettings.AudioSettings.SFXVolume);
    AudioJson->SetNumberField(TEXT("AmbientVolume"), AllSettings.AudioSettings.AmbientVolume);
    JsonObject->SetObjectField(TEXT("Audio"), AudioJson);

    // Gameplay settings
    TSharedPtr<FJsonObject> GameplayJson = MakeShareable(new FJsonObject);
    GameplayJson->SetNumberField(TEXT("DifficultyLevel"), static_cast<int32>(AllSettings.GameplaySettings.DifficultyLevel));
    GameplayJson->SetNumberField(TEXT("MouseSensitivity"), AllSettings.GameplaySettings.MouseSensitivity);
    GameplayJson->SetBoolField(TEXT("PuzzleHintsEnabled"), AllSettings.GameplaySettings.bPuzzleHintSystemEnabled);
    JsonObject->SetObjectField(TEXT("Gameplay"), GameplayJson);

    // Convert to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to serialize settings to JSON"));
        return false;
    }

    // Write to file
    if (!FFileHelper::SaveStringToFile(OutputString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to write settings to file"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Settings exported successfully"));
    return true;
}

bool USettingsSubsystem::ImportSettings(const FString& FilePath)
{
    FScopeLock Lock(&SettingsMutex);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Importing settings from %s"), *FilePath);

    // Read file
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to read file"));
        return false;
    }

    // Parse JSON
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to parse JSON"));
        return false;
    }

    // Check version
    int32 ImportVersion = JsonObject->GetIntegerField(TEXT("Version"));
    if (ImportVersion > CurrentSettingsVersion)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Settings file version too new (%d > %d)"),
            ImportVersion, CurrentSettingsVersion);
        return false;
    }

    // Create new settings object
    FS_AllSettings ImportedSettings;

    // Parse graphics settings
    TSharedPtr<FJsonObject> GraphicsJson = JsonObject->GetObjectField(TEXT("Graphics"));
    if (GraphicsJson.IsValid())
    {
        ImportedSettings.GraphicsSettings.QualityPreset =
            static_cast<EE_GraphicsQuality>(GraphicsJson->GetIntegerField(TEXT("QualityPreset")));
        ImportedSettings.GraphicsSettings.ResolutionX = GraphicsJson->GetIntegerField(TEXT("ResolutionX"));
        ImportedSettings.GraphicsSettings.ResolutionY = GraphicsJson->GetIntegerField(TEXT("ResolutionY"));
        ImportedSettings.GraphicsSettings.bVSyncEnabled = GraphicsJson->GetBoolField(TEXT("VSyncEnabled"));
        ImportedSettings.GraphicsSettings.FrameRateCap = GraphicsJson->GetIntegerField(TEXT("FrameRateCap"));
        ImportedSettings.GraphicsSettings.bRayTracingEnabled = GraphicsJson->GetBoolField(TEXT("RayTracingEnabled"));
        ImportedSettings.GraphicsSettings.FieldOfView = GraphicsJson->GetNumberField(TEXT("FieldOfView"));
    }

    // Parse audio settings
    TSharedPtr<FJsonObject> AudioJson = JsonObject->GetObjectField(TEXT("Audio"));
    if (AudioJson.IsValid())
    {
        ImportedSettings.AudioSettings.MasterVolume = AudioJson->GetNumberField(TEXT("MasterVolume"));
        ImportedSettings.AudioSettings.MusicVolume = AudioJson->GetNumberField(TEXT("MusicVolume"));
        ImportedSettings.AudioSettings.SFXVolume = AudioJson->GetNumberField(TEXT("SFXVolume"));
        ImportedSettings.AudioSettings.AmbientVolume = AudioJson->GetNumberField(TEXT("AmbientVolume"));
    }

    // Parse gameplay settings
    TSharedPtr<FJsonObject> GameplayJson = JsonObject->GetObjectField(TEXT("Gameplay"));
    if (GameplayJson.IsValid())
    {
        ImportedSettings.GameplaySettings.DifficultyLevel =
            static_cast<EE_DifficultyLevel>(GameplayJson->GetIntegerField(TEXT("DifficultyLevel")));
        ImportedSettings.GameplaySettings.MouseSensitivity = GameplayJson->GetNumberField(TEXT("MouseSensitivity"));
        ImportedSettings.GameplaySettings.bPuzzleHintSystemEnabled = GameplayJson->GetBoolField(TEXT("PuzzleHintsEnabled"));
    }

    // Validate imported settings
    TArray<FString> Errors, Warnings;
    if (!ValidateSettings(ImportedSettings, Errors, Warnings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Imported settings failed validation"));
        return false;
    }

    // Apply imported settings
    return ApplyAllSettings(ImportedSettings, false, false);
}

// ===== RESET TO DEFAULT =====

void USettingsSubsystem::ResetSettingsToDefault(ESettingsCategory Category)
{
    FScopeLock Lock(&SettingsMutex);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Resetting settings to default (Category: %d)"),
        static_cast<int32>(Category));

    FS_AllSettings DefaultSettings;

    switch (Category)
    {
    case ESettingsCategory::Graphics:
        AllSettings.GraphicsSettings = GetGraphicsPreset(EE_GraphicsQuality::Medium);
        ApplyGraphicsSettings(AllSettings.GraphicsSettings);
        break;

    case ESettingsCategory::Audio:
        AllSettings.AudioSettings = DefaultSettings.AudioSettings;
        ApplyAudioSettings(AllSettings.AudioSettings);
        break;

    case ESettingsCategory::Gameplay:
        AllSettings.GameplaySettings = DefaultSettings.GameplaySettings;
        ApplyGameplaySettings(AllSettings.GameplaySettings);
        break;

    case ESettingsCategory::Controls:
        AllSettings.ControlSettings = DefaultSettings.ControlSettings;
        ApplyControlSettings(AllSettings.ControlSettings);
        break;

    case ESettingsCategory::Accessibility:
        AllSettings.AccessibilitySettings = DefaultSettings.AccessibilitySettings;
        ApplyAccessibilitySettings(AllSettings.AccessibilitySettings);
        break;
    case ESettingsCategory::All:
    default:
        AllSettings = DefaultSettings;
        ApplyAllSettings(AllSettings, false, false);
        break;
    }

    SaveAllSettings(true);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Settings reset complete"));
}

// ===== CUSTOM PRESETS =====

bool USettingsSubsystem::SaveCustomPreset(const FString& PresetName)
{
    FScopeLock Lock(&SettingsMutex);

    if (PresetName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Preset name cannot be empty"));
        return false;
    }

    // Validate current settings before saving as preset
    TArray<FString> Errors, Warnings;
    if (!ValidateSettings(AllSettings, Errors, Warnings))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Cannot save invalid settings as preset"));
        return false;
    }

    // Save to custom presets map
    CustomPresets.Add(PresetName, AllSettings);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Custom preset '%s' saved"), *PresetName);

    // Persist custom presets to disk
    return SaveCustomPresetsToDisk();
}

bool USettingsSubsystem::LoadCustomPreset(const FString& PresetName)
{
    FScopeLock Lock(&SettingsMutex);

    FS_AllSettings* PresetSettings = CustomPresets.Find(PresetName);

    if (!PresetSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Custom preset '%s' not found"), *PresetName);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Loading custom preset '%s'"), *PresetName);

    return ApplyAllSettings(*PresetSettings, false, false);
}

bool USettingsSubsystem::DeleteCustomPreset(const FString& PresetName)
{
    FScopeLock Lock(&SettingsMutex);

    int32 RemovedCount = CustomPresets.Remove(PresetName);

    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Custom preset '%s' deleted"), *PresetName);
        return SaveCustomPresetsToDisk();
    }

    UE_LOG(LogTemp, Warning, TEXT("SettingsSubsystem: Custom preset '%s' not found"), *PresetName);
    return false;
}

TArray<FString> USettingsSubsystem::GetCustomPresetNames() const
{
    TArray<FString> Names;
    CustomPresets.GetKeys(Names);
    return Names;
}

bool USettingsSubsystem::SaveCustomPresetsToDisk()
{
    FScopeLock Lock(&SettingsMutex);

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Saving custom presets to disk"));

    TSharedPtr<FJsonObject> RootJson = MakeShareable(new FJsonObject);

    for (const auto& Pair : CustomPresets)
    {
        const FString& PresetName = Pair.Key;
        const FS_AllSettings& PresetSettings = Pair.Value;

        TSharedPtr<FJsonObject> PresetJson = MakeShareable(new FJsonObject);

        // Graphics
        TSharedPtr<FJsonObject> G = MakeShareable(new FJsonObject);
        G->SetNumberField(TEXT("QualityPreset"), static_cast<int32>(PresetSettings.GraphicsSettings.QualityPreset));
        G->SetNumberField(TEXT("ResolutionX"), PresetSettings.GraphicsSettings.ResolutionX);
        G->SetNumberField(TEXT("ResolutionY"), PresetSettings.GraphicsSettings.ResolutionY);
        G->SetBoolField(TEXT("VSyncEnabled"), PresetSettings.GraphicsSettings.bVSyncEnabled);
        G->SetNumberField(TEXT("FrameRateCap"), PresetSettings.GraphicsSettings.FrameRateCap);
        G->SetBoolField(TEXT("RayTracingEnabled"), PresetSettings.GraphicsSettings.bRayTracingEnabled);
        G->SetNumberField(TEXT("FieldOfView"), PresetSettings.GraphicsSettings.FieldOfView);
        G->SetNumberField(TEXT("TextureQuality"), static_cast<int32>(PresetSettings.GraphicsSettings.TextureQuality));
        G->SetNumberField(TEXT("ShadowQuality"), static_cast<int32>(PresetSettings.GraphicsSettings.ShadowQuality));
        G->SetNumberField(TEXT("AntiAliasingMethod"), static_cast<int32>(PresetSettings.GraphicsSettings.AntiAliasingMethod));
        G->SetNumberField(TEXT("MotionBlurAmount"), PresetSettings.GraphicsSettings.MotionBlurAmount);
        PresetJson->SetObjectField(TEXT("Graphics"), G);

        // Audio
        TSharedPtr<FJsonObject> A = MakeShareable(new FJsonObject);
        A->SetNumberField(TEXT("MasterVolume"), PresetSettings.AudioSettings.MasterVolume);
        A->SetNumberField(TEXT("MusicVolume"), PresetSettings.AudioSettings.MusicVolume);
        A->SetNumberField(TEXT("SFXVolume"), PresetSettings.AudioSettings.SFXVolume);
        A->SetNumberField(TEXT("AmbientVolume"), PresetSettings.AudioSettings.AmbientVolume);
        PresetJson->SetObjectField(TEXT("Audio"), A);

        // Gameplay
        TSharedPtr<FJsonObject> P = MakeShareable(new FJsonObject);
        P->SetNumberField(TEXT("DifficultyLevel"), static_cast<int32>(PresetSettings.GameplaySettings.DifficultyLevel));
        P->SetNumberField(TEXT("MouseSensitivity"), PresetSettings.GameplaySettings.MouseSensitivity);
        P->SetBoolField(TEXT("PuzzleHintsEnabled"), PresetSettings.GameplaySettings.bPuzzleHintSystemEnabled);
        PresetJson->SetObjectField(TEXT("Gameplay"), P);

        // Controls (example fields; adapt to your FS_ControlSettings)
        TSharedPtr<FJsonObject> C = MakeShareable(new FJsonObject);
        // Add control fields as needed...
        PresetJson->SetObjectField(TEXT("Controls"), C);

        // Accessibility (example)
        TSharedPtr<FJsonObject> ACC = MakeShareable(new FJsonObject);
        ACC->SetNumberField(TEXT("HoldActivationTime"), PresetSettings.AccessibilitySettings.HoldActivationTime);
        PresetJson->SetObjectField(TEXT("Accessibility"), ACC);

        RootJson->SetObjectField(PresetName, PresetJson);
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    if (!FJsonSerializer::Serialize(RootJson.ToSharedRef(), Writer))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to serialize custom presets to JSON"));
        return false;
    }

    const FString FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("CustomPresets.json"));
    if (!FFileHelper::SaveStringToFile(OutputString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to write custom presets file: %s"), *FilePath);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Custom presets saved to %s"), *FilePath);
    return true;
}

void USettingsSubsystem::LoadCustomPresetsFromDisk()
{
    FScopeLock Lock(&SettingsMutex);

    const FString FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("CustomPresets.json"));

    if (!FPaths::FileExists(FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: No custom presets file found at %s"), *FilePath);
        return;
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to load custom presets file: %s"), *FilePath);
        return;
    }

    TSharedPtr<FJsonObject> RootJson;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootJson) || !RootJson.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Failed to parse custom presets JSON"));
        return;
    }

    CustomPresets.Empty();

    for (const auto& Pair : RootJson->Values)
    {
        const FString PresetName = Pair.Key;
        TSharedPtr<FJsonObject> PresetJson = Pair.Value->AsObject();
        if (!PresetJson.IsValid())
            continue;

        FS_AllSettings Settings;

        // Graphics
        TSharedPtr<FJsonObject> G = PresetJson->GetObjectField(TEXT("Graphics"));
        if (G.IsValid())
        {
            Settings.GraphicsSettings.QualityPreset = static_cast<EE_GraphicsQuality>(G->GetIntegerField(TEXT("QualityPreset")));
            Settings.GraphicsSettings.ResolutionX = G->GetIntegerField(TEXT("ResolutionX"));
            Settings.GraphicsSettings.ResolutionY = G->GetIntegerField(TEXT("ResolutionY"));
            Settings.GraphicsSettings.bVSyncEnabled = G->GetBoolField(TEXT("VSyncEnabled"));
            Settings.GraphicsSettings.FrameRateCap = G->GetIntegerField(TEXT("FrameRateCap"));
            Settings.GraphicsSettings.bRayTracingEnabled = G->GetBoolField(TEXT("RayTracingEnabled"));
            Settings.GraphicsSettings.FieldOfView = G->GetNumberField(TEXT("FieldOfView"));
            if (G->HasField(TEXT("TextureQuality")))
            {
                int32 IntVal = G->GetIntegerField(TEXT("TextureQuality"));
                using TextureQualityType = decltype(Settings.GraphicsSettings.TextureQuality);
                Settings.GraphicsSettings.TextureQuality = static_cast<TextureQualityType>(IntVal);
            }

            if (G->HasField(TEXT("ShadowQuality")))
            {
                int32 IntVal = G->GetIntegerField(TEXT("ShadowQuality"));
                using ShadowQualityType = decltype(Settings.GraphicsSettings.ShadowQuality);
                Settings.GraphicsSettings.ShadowQuality = static_cast<ShadowQualityType>(IntVal);
            }

            if (G->HasField(TEXT("AntiAliasingMethod")))
            {
                int32 IntVal = G->GetIntegerField(TEXT("AntiAliasingMethod"));
                using AAMethodType = decltype(Settings.GraphicsSettings.AntiAliasingMethod);
                Settings.GraphicsSettings.AntiAliasingMethod = static_cast<AAMethodType>(IntVal);
            }

            if (G->HasField(TEXT("MotionBlurAmount")))
            {
                Settings.GraphicsSettings.MotionBlurAmount = G->GetNumberField(TEXT("MotionBlurAmount"));
            }
        }

        // Audio
        TSharedPtr<FJsonObject> A = PresetJson->GetObjectField(TEXT("Audio"));
        if (A.IsValid())
        {
            Settings.AudioSettings.MasterVolume = A->GetNumberField(TEXT("MasterVolume"));
            Settings.AudioSettings.MusicVolume = A->GetNumberField(TEXT("MusicVolume"));
            Settings.AudioSettings.SFXVolume = A->GetNumberField(TEXT("SFXVolume"));
            Settings.AudioSettings.AmbientVolume = A->GetNumberField(TEXT("AmbientVolume"));
        }

        // Gameplay
        TSharedPtr<FJsonObject> P = PresetJson->GetObjectField(TEXT("Gameplay"));
        if (P.IsValid())
        {
            Settings.GameplaySettings.DifficultyLevel = static_cast<EE_DifficultyLevel>(P->GetIntegerField(TEXT("DifficultyLevel")));
            Settings.GameplaySettings.MouseSensitivity = P->GetNumberField(TEXT("MouseSensitivity"));
            Settings.GameplaySettings.bPuzzleHintSystemEnabled = P->GetBoolField(TEXT("PuzzleHintsEnabled"));
        }

        // Accessibility
        TSharedPtr<FJsonObject> ACC = PresetJson->GetObjectField(TEXT("Accessibility"));
        if (ACC.IsValid())
        {
            if (ACC->HasField(TEXT("HoldActivationTime")))
            {
                Settings.AccessibilitySettings.HoldActivationTime = ACC->GetNumberField(TEXT("HoldActivationTime"));
            }
        }

        CustomPresets.Add(PresetName, Settings);
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Loaded %d custom presets from %s"), CustomPresets.Num(), *FilePath);
}

TArray<EE_GraphicsQuality> USettingsSubsystem::GetAvailableGraphicsPresets() const
{
    return FGraphicsSettingsHandler::GetAvailablePresets();
}

// ===== PERFORMANCE METRICS =====

FS_PerformanceMetrics USettingsSubsystem::GetPerformanceMetrics() const
{
      float CurrentTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());

    // Return cached if still valid
    if (CurrentTime - LastMetricsCacheTime < MetricsCacheDuration)
    {
        return CachedMetrics;
    }

    FS_PerformanceMetrics Metrics;

    if (!GEngine || !GetWorld())
    {
        return Metrics;
    }

    // ===== FPS MEASUREMENT =====
    extern ENGINE_API float GAverageFPS;
    
    // Method 1: Global average FPS
    if (GAverageFPS > 0.0f)
    {
        Metrics.AverageFPS = GAverageFPS;
        Metrics.AverageFrameTime = 1000.0f / GAverageFPS;
    }
    // Method 2: Calculate from delta time
    else
    {
        float DeltaTime = GetWorld()->GetDeltaSeconds();
        if (DeltaTime > 0.0f)
        {
            Metrics.AverageFPS = 1.0f / DeltaTime;
            Metrics.AverageFrameTime = DeltaTime * 1000.0f;
        }
    }

    // ===== RENDERING STATS =====
    
    if (FModuleManager::Get().IsModuleLoaded(TEXT("Renderer")))
    {
        // Lấy module renderer (không phải từ GEngine)
        IRendererModule& RendererModule = FModuleManager::GetModuleChecked<IRendererModule>(TEXT("Renderer"));

        // NOTE:
        // - Engine không expose trực tiếp API "GetDrawCallCount" hay "GetTriangleCount" ở mức public.
        // - Việc lấy drawcalls/triangles thực sự yêu cầu truy vấn các thống kê nội bộ của renderer
        //   (hoặc dùng hệ thống thống kê RHI / stat commands) và có thể khác giữa UE4/UE5.
        //
        // Ở đây ta giữ giá trị mặc định (0) và ghi chú; nếu bạn cần con số thật, mình sẽ bổ sung
        // logic cụ thể theo phiên bản Unreal (ví dụ đọc từ RHI stats hoặc dùng renderer debug interface).
        Metrics.DrawCalls = 0;
        Metrics.Triangles = 0;
    }
    else
    {
        Metrics.DrawCalls = 0;
        Metrics.Triangles = 0;
    }

    // ===== MEMORY STATS =====
    
    // RAM usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.UsedRAMMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);

    // VRAM usage (approximate from texture memory)
    if (GDynamicRHI)
    {
        FTextureMemoryStats TextureStats;
        RHIGetTextureMemoryStats(TextureStats);
        
        if (TextureStats.DedicatedVideoMemory > 0)
        {
            Metrics.UsedVRAMMB = static_cast<float>(TextureStats.DedicatedVideoMemory) / (1024.0f * 1024.0f);
        }
    }

    Metrics.MeasurementTime = FDateTime::UtcNow();

    // Cache results
    CachedMetrics = Metrics;
    LastMetricsCacheTime = CurrentTime;

    return Metrics;
}

// ===== DIAGNOSTICS =====

FS_DifficultyMultiplier USettingsSubsystem::GetDifficultyMultiplier(EE_DifficultyLevel DifficultyLevel) const
{
    return FS_DifficultyMultiplier();
}

FString USettingsSubsystem::RunDiagnostics()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Running diagnostics..."));

    FString Report = TEXT("===== SETTINGS SYSTEM DIAGNOSTICS =====\n\n");

    // System Info
    Report += TEXT("--- SYSTEM INFORMATION ---\n");
    Report += FString::Printf(TEXT("OS: %s\n"), *HardwareInfo.OperatingSystem);
    Report += FString::Printf(TEXT("GPU: %s (%s)\n"), *HardwareInfo.GPU.Name, *HardwareInfo.GPU.Vendor);
    Report += FString::Printf(TEXT("VRAM: %d MB\n"), HardwareInfo.GPU.VRAMSizeMB);
    Report += FString::Printf(TEXT("CPU: %s\n"), *HardwareInfo.CPU.Name);
    Report += FString::Printf(TEXT("Cores: %d (%d threads)\n"),
        HardwareInfo.CPU.CoreCount, HardwareInfo.CPU.ThreadCount);
    Report += FString::Printf(TEXT("RAM: %d GB\n"), HardwareInfo.Memory.TotalPhysicalGB);
    Report += FString::Printf(TEXT("Ray Tracing: %s\n"),
        HardwareInfo.GPU.bSupportsRayTracing ? TEXT("Supported") : TEXT("Not Supported"));
    Report += TEXT("\n");

    // Current Settings
    Report += TEXT("--- CURRENT SETTINGS ---\n");
    Report += FString::Printf(TEXT("Graphics Quality: %d\n"),
        static_cast<int32>(AllSettings.GraphicsSettings.QualityPreset));
    Report += FString::Printf(TEXT("Resolution: %dx%d\n"),
        AllSettings.GraphicsSettings.ResolutionX, AllSettings.GraphicsSettings.ResolutionY);
    Report += FString::Printf(TEXT("Frame Rate Cap: %d\n"), AllSettings.GraphicsSettings.FrameRateCap);
    Report += FString::Printf(TEXT("VSync: %s\n"),
        AllSettings.GraphicsSettings.bVSyncEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    Report += FString::Printf(TEXT("Ray Tracing: %s\n"),
        AllSettings.GraphicsSettings.bRayTracingEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    Report += FString::Printf(TEXT("FOV: %.1f\n"), AllSettings.GraphicsSettings.FieldOfView);
    Report += TEXT("\n");

    // Performance Metrics
    FS_PerformanceMetrics Metrics = GetPerformanceMetrics();
    Report += TEXT("--- PERFORMANCE METRICS ---\n");
    Report += FString::Printf(TEXT("Average FPS: %.1f\n"), Metrics.AverageFPS);
    Report += FString::Printf(TEXT("Frame Time: %.2f ms\n"), Metrics.AverageFrameTime);
    Report += FString::Printf(TEXT("RAM Usage: %.1f MB\n"), Metrics.UsedRAMMB);
    Report += FString::Printf(TEXT("VRAM Usage: %.1f MB\n"), Metrics.UsedVRAMMB);
    Report += TEXT("\n");

    // Validation Check
    TArray<FString> Errors, Warnings;
    bool bValid = ValidateSettings(AllSettings, Errors, Warnings);

    Report += TEXT("--- VALIDATION STATUS ---\n");
    Report += FString::Printf(TEXT("Status: %s\n"), bValid ? TEXT("PASSED") : TEXT("FAILED"));

    if (Errors.Num() > 0)
    {
        Report += FString::Printf(TEXT("Errors: %d\n"), Errors.Num());
        for (const FString& Error : Errors)
        {
            Report += FString::Printf(TEXT("  - %s\n"), *Error);
        }
    }

    if (Warnings.Num() > 0)
    {
        Report += FString::Printf(TEXT("Warnings: %d\n"), Warnings.Num());
        for (const FString& Warning : Warnings)
        {
            Report += FString::Printf(TEXT("  - %s\n"), *Warning);
        }
    }

    Report += TEXT("\n");

    // Recommendations
    Report += TEXT("--- RECOMMENDATIONS ---\n");

    if (HardwareInfo.OverallPerformanceScore < 50)
    {
        Report += TEXT("- Consider lowering graphics quality for better performance\n");
    }

    if (AllSettings.GraphicsSettings.bRayTracingEnabled && !HardwareInfo.GPU.bSupportsRayTracing)
    {
        Report += TEXT("- Ray tracing is enabled but not supported by your GPU\n");
    }

    if (Metrics.AverageFPS < 30.0f)
    {
        Report += TEXT("- Performance is below 30 FPS, consider reducing settings\n");
    }

    if (EstimateVRAMUsage(AllSettings.GraphicsSettings) > HardwareInfo.GPU.VRAMSizeMB)
    {
        Report += TEXT("- Settings may exceed available VRAM\n");
    }

    Report += TEXT("\n");

    // Save State
    Report += TEXT("--- SAVE STATE ---\n");
    Report += FString::Printf(TEXT("Dirty Flag: %s\n"), bDirtyFlag ? TEXT("Modified") : TEXT("Clean"));
    Report += FString::Printf(TEXT("Settings Version: %d\n"), CurrentSettingsVersion);
    Report += TEXT("\n");

    Report += TEXT("=== END DIAGNOSTICS ===\n");

    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Diagnostics complete"));

    return Report;
}

// ===== BENCHMARKING =====

void USettingsSubsystem::BenchmarkGraphicsSettings(float Duration)
{
    FGraphicsSettingsHandler::BenchmarkSettings(Duration, GetWorld(),
        [](EE_GraphicsQuality Quality, float FPS)
        {
            UE_LOG(LogTemp, Log, TEXT("Benchmark result - %s: %.1f FPS"),
                *UEnum::GetValueAsString(Quality), FPS);
        });
}

// ===== HELPER FUNCTIONS =====

bool USettingsSubsystem::ApplySettingsCategory(ESettingsCategory Category, bool bAsync)
{
    switch (Category)
    {
    case ESettingsCategory::Graphics:
        return ApplyGraphicsSettingsToEngine(AllSettings.GraphicsSettings);

    case ESettingsCategory::Audio:
        return ApplyAudioSettingsToEngine(AllSettings.AudioSettings);

    case ESettingsCategory::Gameplay:
        return ApplyGameplaySettingsToEngine(AllSettings.GameplaySettings);

    case ESettingsCategory::Controls:
        return ApplyControlSettingsToEngine(AllSettings.ControlSettings);

    case ESettingsCategory::Accessibility:
        return ApplyAccessibilitySettingsToEngine(AllSettings.AccessibilitySettings);
    default:
        return ApplyAllSettings(AllSettings, bAsync, false);
    }
}

// ===== IMPROVED APPLY METHODS (with error handling) =====

FS_GraphicsSettings USettingsSubsystem::GetGraphicsPreset(EE_GraphicsQuality Quality) const
{
    return FGraphicsSettingsHandler::GetPreset(Quality);
}

void USettingsSubsystem::InitGraphicSettings()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Initializing graphic presets"));

    GraphicsPresets.Empty();

    GraphicsPresets.Add(EE_GraphicsQuality::Low, GetGraphicsPreset(EE_GraphicsQuality::Low));
    GraphicsPresets.Add(EE_GraphicsQuality::Medium, GetGraphicsPreset(EE_GraphicsQuality::Medium));
    GraphicsPresets.Add(EE_GraphicsQuality::High, GetGraphicsPreset(EE_GraphicsQuality::High));
    GraphicsPresets.Add(EE_GraphicsQuality::Ultra, GetGraphicsPreset(EE_GraphicsQuality::Ultra));
    GraphicsPresets.Add(EE_GraphicsQuality::Custom, GetGraphicsPreset(EE_GraphicsQuality::Custom));
}

bool USettingsSubsystem::ApplyGraphicsSettings(const FS_GraphicsSettings& Settings)
{
    const bool bResult = ApplyGraphicsSettingsToEngine(Settings);
    if (bResult)
    {
        AllSettings.GraphicsSettings = Settings;
        OnGraphicsSettingsChanged.Broadcast(AllSettings.GraphicsSettings);
        bDirtyFlag = true;
    }
    return bResult;
}

bool USettingsSubsystem::ApplyAudioSettings(const FS_AudioSettings& Settings)
{
    const bool bResult = ApplyAudioSettingsToEngine(Settings);
    if (bResult)
    {
        AllSettings.AudioSettings = Settings;
        OnAudioSettingsChanged.Broadcast(AllSettings.AudioSettings);
        bDirtyFlag = true;
    }
    return bResult;
}

bool USettingsSubsystem::ApplyGameplaySettings(const FS_GameplaySettings& Settings)
{
    const bool bResult = ApplyGameplaySettingsToEngine(Settings);
    if (bResult)
    {
        AllSettings.GameplaySettings = Settings;
        OnGameplaySettingsChanged.Broadcast(AllSettings.GameplaySettings);
        bDirtyFlag = true;
    }
    return bResult;
}

bool USettingsSubsystem::ApplyControlSettings(const FS_ControlSettings& Settings)
{
    const bool bResult = ApplyControlSettingsToEngine(Settings);
    if (bResult)
    {
        AllSettings.ControlSettings = Settings;
        OnControlSettingsChanged.Broadcast(AllSettings.ControlSettings);
        bDirtyFlag = true;
    }
    return bResult;
}

bool USettingsSubsystem::ApplyAccessibilitySettings(const FS_AccessibilitySettings& Settings)
{
    const bool bResult = ApplyAccessibilitySettingsToEngine(Settings);
    if (bResult)
    {
        AllSettings.AccessibilitySettings = Settings;
        OnAccessibilitySettingsChanged.Broadcast(AllSettings.AccessibilitySettings);
        bDirtyFlag = true;
    }
    return bResult;
}

bool USettingsSubsystem::ApplyGraphicsSettingsToEngine(const FS_GraphicsSettings& Settings)
{
    try
    {
        FGraphicsSettingsHandler::ApplyToEngine(Settings, GetWorld());
        return true;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Exception while applying graphics settings"));
        return false;
    }
}

bool USettingsSubsystem::ApplyAudioSettingsToEngine(const FS_AudioSettings& Settings)
{
    try
    {
        FAudioSettingsHandler::Initialize(GetWorld());
        FAudioSettingsHandler::ApplyToEngine(Settings, GetWorld());
        return true;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Exception while applying audio settings"));
        return false;
    }
}

bool USettingsSubsystem::ApplyGameplaySettingsToEngine(const FS_GameplaySettings& Settings)
{
    try
    {
        FGameplaySettingsHandler::ApplyToEngine(Settings, GetWorld());
        return true;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Exception while applying gameplay settings"));
        return false;
    }
}

bool USettingsSubsystem::ApplyControlSettingsToEngine(const FS_ControlSettings& Settings)
{
    try
    {
        FControlSettingsHandler::ApplyToEngine(Settings, GetWorld());
        return true;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem: Exception while applying control settings"));
        return false;
    }
}

bool USettingsSubsystem::ApplyAccessibilitySettingsToEngine(const FS_AccessibilitySettings& Settings)
{
    try
    {
        FAccessibilitySettingsHandler::Initialize(GetWorld());
        FAccessibilitySettingsHandler::ApplyToEngine(Settings,GetWorld());
        return true;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Log, TEXT("SettingsSubsystem:Exception while applying Accessibility settings"));
        return false;
    }
}