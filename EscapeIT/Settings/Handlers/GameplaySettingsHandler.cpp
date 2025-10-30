#include "EscapeIT/Settings/Handlers/GameplaySettingsHandler.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"

// Static member initialization
FS_GameplaySettings FGameplaySettingsHandler::CurrentSettings;
bool FGameplaySettingsHandler::bIsInitialized = false;

// ===== INITIALIZATION =====

void FGameplaySettingsHandler::Initialize(UWorld* World)
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplaySettingsHandler: Already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Initializing..."));

    // Set default values
    CurrentSettings = FS_GameplaySettings();

    // Apply defaults
    ApplyToEngine(CurrentSettings, World);

    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Initialized successfully"));
}

// ===== MAIN APPLY FUNCTION =====

void FGameplaySettingsHandler::ApplyToEngine(const FS_GameplaySettings& Settings, UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GameplaySettingsHandler: World is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Applying gameplay settings..."));

    // Store current settings
    CurrentSettings = Settings;

    // Apply difficulty
    SetDifficulty(Settings.DifficultyLevel, World);

    // Apply sensitivity
    SetMouseSensitivity(Settings.MouseSensitivity, World);

    // Apply multipliers
    SetSanityDrainMultiplier(Settings.SanityDrainMultiplier);
    SetEntityDetectionRange(Settings.EntityDetectionRangeMultiplier);

    // Apply hint system
    SetPuzzleHintsEnabled(Settings.bPuzzleHintSystemEnabled);
    SetAutoRevealHintTime(Settings.AutoRevealHintTime);
    SetAllowSkipPuzzles(Settings.bAllowSkipPuzzles);

    // Apply UI settings
    SetShowObjectiveMarkers(Settings.bShowObjectiveMarkers);
    SetShowInteractionIndicators(Settings.bShowInteractionIndicators);
    SetAutoPickupItems(Settings.bAutoPickupItems);

    // Apply camera effects
    SetCameraShakeMagnitude(Settings.CameraShakeMagnitude, World);
    SetScreenBlurAmount(Settings.ScreenBlurAmount, World);

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Settings applied successfully"));
}

// ===== DIFFICULTY SYSTEM =====

void FGameplaySettingsHandler::SetDifficulty(EE_DifficultyLevel Difficulty, UWorld* World)
{
    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Setting difficulty to %d"),
        static_cast<int32>(Difficulty));

    CurrentSettings.DifficultyLevel = Difficulty;

    // Apply difficulty multipliers
    ApplyDifficultyMultipliers(Difficulty, World);

    // Broadcast difficulty change (you can implement this via GameInstance or GameMode)
    // Example: UGameInstance->OnDifficultyChanged.Broadcast(Difficulty);
}

void FGameplaySettingsHandler::ApplyDifficultyMultipliers(EE_DifficultyLevel Difficulty, UWorld* World)
{
    FS_DifficultyMultiplier Multipliers = GetDifficultyMultipliers(Difficulty);

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Applying difficulty multipliers"));
    UE_LOG(LogTemp, Log, TEXT("  - Sanity Drain: %.2fx"), Multipliers.SanityDrainMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  - AI Detection: %.2fx"), Multipliers.AIDetectionMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  - AI Speed: %.2fx"), Multipliers.AISpeedMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  - Entity Chase: %.2fx"), Multipliers.EntityChaseMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  - Battery Life: %.2fx"), Multipliers.BatteryLifeMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  - Hint Availability: %.2fx"), Multipliers.PuzzleHintAvailability);

    // Store in game instance or game mode for AI/systems to access
    // Example implementation:
    /*
    if (AEscapeITGameMode* GameMode = Cast<AEscapeITGameMode>(World->GetAuthGameMode()))
    {
        GameMode->SetDifficultyMultipliers(Multipliers);
    }
    */
}

FS_DifficultyMultiplier FGameplaySettingsHandler::GetDifficultyMultipliers(EE_DifficultyLevel Difficulty)
{
    FS_DifficultyMultiplier Multipliers;

    switch (Difficulty)
    {
    case EE_DifficultyLevel::Easy:
        Multipliers.SanityDrainMultiplier = 0.5f;      // Slower sanity drain
        Multipliers.AIDetectionMultiplier = 0.7f;       // Enemies detect less
        Multipliers.AISpeedMultiplier = 0.8f;           // Slower enemies
        Multipliers.EntityChaseMultiplier = 0.75f;      // Shorter chase time
        Multipliers.BatteryLifeMultiplier = 1.5f;       // Longer battery
        Multipliers.PuzzleHintAvailability = 1.5f;      // More hints
        break;

    case EE_DifficultyLevel::Normal:
        Multipliers.SanityDrainMultiplier = 1.0f;
        Multipliers.AIDetectionMultiplier = 1.0f;
        Multipliers.AISpeedMultiplier = 1.0f;
        Multipliers.EntityChaseMultiplier = 1.0f;
        Multipliers.BatteryLifeMultiplier = 1.0f;
        Multipliers.PuzzleHintAvailability = 1.0f;
        break;

    case EE_DifficultyLevel::Hard:
        Multipliers.SanityDrainMultiplier = 1.5f;       // Faster sanity drain
        Multipliers.AIDetectionMultiplier = 1.3f;       // Enemies detect more
        Multipliers.AISpeedMultiplier = 1.2f;           // Faster enemies
        Multipliers.EntityChaseMultiplier = 1.5f;       // Longer chase time
        Multipliers.BatteryLifeMultiplier = 0.7f;       // Shorter battery
        Multipliers.PuzzleHintAvailability = 0.7f;      // Fewer hints
        break;

    case EE_DifficultyLevel::Nightmare:
        Multipliers.SanityDrainMultiplier = 2.0f;       // Much faster drain
        Multipliers.AIDetectionMultiplier = 1.5f;       // Very alert enemies
        Multipliers.AISpeedMultiplier = 1.4f;           // Much faster enemies
        Multipliers.EntityChaseMultiplier = 2.0f;       // Very long chase
        Multipliers.BatteryLifeMultiplier = 0.5f;       // Very short battery
        Multipliers.PuzzleHintAvailability = 0.5f;      // Very few hints
        break;
    }

    return Multipliers;
}

// ===== SENSITIVITY =====

void FGameplaySettingsHandler::SetMouseSensitivity(float Sensitivity, UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GameplaySettingsHandler: World is null"));
        return;
    }

    Sensitivity = FMath::Clamp(Sensitivity, 0.1f, 10.0f);
    CurrentSettings.MouseSensitivity = Sensitivity;

    UpdatePlayerSensitivity(Sensitivity, World);

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Mouse sensitivity set to %.2f"), Sensitivity);
}

void FGameplaySettingsHandler::UpdatePlayerSensitivity(float Sensitivity, UWorld* World)
{
    if (!World) return;

    // Apply to all player controllers
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->PlayerInput)
        {
            // Mouse X/Y axis sensitivity
            for (FInputAxisConfigEntry& AxisConfig : PC->PlayerInput->AxisConfig)
            {
                if (AxisConfig.AxisKeyName == EKeys::MouseX ||
                    AxisConfig.AxisKeyName == EKeys::MouseY)
                {
                    AxisConfig.AxisProperties.Sensitivity = Sensitivity;
                }
            }

            UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Applied sensitivity to player %d"),
                PC->GetLocalPlayer()->GetControllerId());
        }
    }
}

// ===== MULTIPLIERS =====

void FGameplaySettingsHandler::SetSanityDrainMultiplier(float Multiplier)
{
    Multiplier = FMath::Clamp(Multiplier, 0.1f, 5.0f);
    CurrentSettings.SanityDrainMultiplier = Multiplier;

    // Store for sanity system to access
    // Example: GameInstance->SanityDrainMultiplier = Multiplier;

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Sanity drain multiplier set to %.2f"), Multiplier);
}

void FGameplaySettingsHandler::SetEntityDetectionRange(float Multiplier)
{
    Multiplier = FMath::Clamp(Multiplier, 0.1f, 5.0f);
    CurrentSettings.EntityDetectionRangeMultiplier = Multiplier;

    // Store for AI system to access
    // Example: GameInstance->EntityDetectionMultiplier = Multiplier;

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Entity detection range multiplier set to %.2f"),
        Multiplier);
}

// ===== HINT SYSTEM =====

void FGameplaySettingsHandler::SetPuzzleHintsEnabled(bool bEnabled)
{
    CurrentSettings.bPuzzleHintSystemEnabled = bEnabled;

    // Enable/disable hint system
    // Example: GameInstance->bHintsEnabled = bEnabled;

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Puzzle hints %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void FGameplaySettingsHandler::SetAutoRevealHintTime(float Time)
{
    Time = FMath::Clamp(Time, 10.0f, 300.0f); // 10 seconds to 5 minutes
    CurrentSettings.AutoRevealHintTime = Time;

    // Store for hint system
    // Example: GameInstance->AutoHintRevealTime = Time;

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Auto-reveal hint time set to %.1f seconds"), Time);
}

void FGameplaySettingsHandler::SetAllowSkipPuzzles(bool bAllow)
{
    CurrentSettings.bAllowSkipPuzzles = bAllow;

    // Store for puzzle system
    // Example: GameInstance->bCanSkipPuzzles = bAllow;

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Skip puzzles %s"),
        bAllow ? TEXT("allowed") : TEXT("not allowed"));
}

// ===== UI HELPERS =====

void FGameplaySettingsHandler::SetShowObjectiveMarkers(bool bShow)
{
    CurrentSettings.bShowObjectiveMarkers = bShow;

    // Update UI system
    // Example: HUDWidget->SetObjectiveMarkersVisible(bShow);

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Objective markers %s"),
        bShow ? TEXT("shown") : TEXT("hidden"));
}

void FGameplaySettingsHandler::SetShowInteractionIndicators(bool bShow)
{
    CurrentSettings.bShowInteractionIndicators = bShow;

    // Update interaction system
    // Example: GameInstance->bShowInteractionUI = bShow;

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Interaction indicators %s"),
        bShow ? TEXT("shown") : TEXT("hidden"));
}

void FGameplaySettingsHandler::SetAutoPickupItems(bool bAuto)
{
    CurrentSettings.bAutoPickupItems = bAuto;

    // Update pickup system
    // Example: GameInstance->bAutoPickup = bAuto;

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Auto-pickup items %s"),
        bAuto ? TEXT("enabled") : TEXT("disabled"));
}

// ===== CAMERA EFFECTS =====

void FGameplaySettingsHandler::SetCameraShakeMagnitude(float Magnitude, UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GameplaySettingsHandler: World is null"));
        return;
    }

    Magnitude = FMath::Clamp(Magnitude, 0.0f, 2.0f);
    CurrentSettings.CameraShakeMagnitude = Magnitude;

    // Apply to all player controllers
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->PlayerCameraManager)
        {
            // Store magnitude for camera shake calculations
            // Example: PC->PlayerCameraManager->SetCameraShakeScale(Magnitude);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Camera shake magnitude set to %.2f"), Magnitude);
}

void FGameplaySettingsHandler::SetScreenBlurAmount(float Amount, UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GameplaySettingsHandler: World is null"));
        return;
    }

    Amount = FMath::Clamp(Amount, 0.0f, 1.0f);
    CurrentSettings.ScreenBlurAmount = Amount;

    // Apply blur via post process
    FString Command = FString::Printf(TEXT("r.DepthOfFieldQuality %.2f"), Amount * 4.0f);
    ExecuteConsoleCommand(World, Command);

    UE_LOG(LogTemp, Log, TEXT("GameplaySettingsHandler: Screen blur amount set to %.2f"), Amount);
}

// ===== HELPER FUNCTIONS =====

void FGameplaySettingsHandler::ExecuteConsoleCommand(UWorld* World, const FString& Command)
{
    if (!World || !GEngine)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplaySettingsHandler: Cannot execute command '%s'"), *Command);
        return;
    }

    GEngine->Exec(World, *Command);
}

// ============================================
// INTEGRATION EXAMPLE - GameInstance.h
// ============================================
/*
UCLASS()
class ESCAPEIT_API UEscapeITGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // Current difficulty settings
    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    EE_DifficultyLevel CurrentDifficulty;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    FS_DifficultyMultiplier DifficultyMultipliers;

    // Gameplay settings
    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    float SanityDrainMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    float EntityDetectionMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    bool bHintsEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    float AutoHintRevealTime;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    bool bCanSkipPuzzles;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    bool bShowObjectiveMarkers;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    bool bShowInteractionUI;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    bool bAutoPickup;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void SetDifficultyMultipliers(const FS_DifficultyMultiplier& Multipliers)
    {
        DifficultyMultipliers = Multipliers;
        OnDifficultyChanged.Broadcast(CurrentDifficulty);
    }

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDifficultyChanged, EE_DifficultyLevel, NewDifficulty);
    UPROPERTY(BlueprintAssignable, Category = "Gameplay")
    FOnDifficultyChanged OnDifficultyChanged;
};
*/

// ============================================
// USAGE EXAMPLE IN SETTINGS SUBSYSTEM
// ============================================
/*
bool USettingsSubsystem::ApplyGameplaySettingsToEngine(const FS_GameplaySettings& Settings)
{
    try
    {
        FGameplaySettingsHandler::ApplyToEngine(Settings, GetWorld());
        return true;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsSubsystem: Exception applying gameplay settings"));
        return false;
    }
}

void USettingsSubsystem::SetDifficulty(EE_DifficultyLevel Difficulty)
{
    AllSettings.GameplaySettings.DifficultyLevel = Difficulty;
    FGameplaySettingsHandler::SetDifficulty(Difficulty, GetWorld());

    OnGameplaySettingsChanged.Broadcast(AllSettings.GameplaySettings);
    bDirtyFlag = true;
}

void USettingsSubsystem::SetMouseSensitivity(float Sensitivity)
{
    AllSettings.GameplaySettings.MouseSensitivity = Sensitivity;
    FGameplaySettingsHandler::SetMouseSensitivity(Sensitivity, GetWorld());

    OnGameplaySettingsChanged.Broadcast(AllSettings.GameplaySettings);
    bDirtyFlag = true;
}

// ... other setters ...
*/