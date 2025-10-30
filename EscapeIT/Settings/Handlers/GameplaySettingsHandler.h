#pragma once
#include "CoreMinimal.h"
#include "EscapeIT/Data/SettingsTypes.h"

class ESCAPEIT_API FGameplaySettingsHandler
{
public:
    // Main apply function
    static void ApplyToEngine(const FS_GameplaySettings& Settings, UWorld* World);

    // Initialize (call once at game start)
    static void Initialize(UWorld* World);

    // Individual setters
    static void SetDifficulty(EE_DifficultyLevel Difficulty, UWorld* World);
    static void SetSanityDrainMultiplier(float Multiplier);
    static void SetMouseSensitivity(float Sensitivity, UWorld* World);
    static void SetEntityDetectionRange(float Multiplier);

    // Hint system
    static void SetPuzzleHintsEnabled(bool bEnabled);
    static void SetAutoRevealHintTime(float Time);
    static void SetAllowSkipPuzzles(bool bAllow);

    // UI helpers
    static void SetShowObjectiveMarkers(bool bShow);
    static void SetShowInteractionIndicators(bool bShow);
    static void SetAutoPickupItems(bool bAuto);

    // Camera effects
    static void SetCameraShakeMagnitude(float Magnitude, UWorld* World);
    static void SetScreenBlurAmount(float Amount, UWorld* World);

    // Get difficulty multipliers
    static FS_DifficultyMultiplier GetDifficultyMultipliers(EE_DifficultyLevel Difficulty);

private:
    // Helper functions
    static void ApplyDifficultyMultipliers(EE_DifficultyLevel Difficulty, UWorld* World);
    static void UpdatePlayerSensitivity(float Sensitivity, UWorld* World);
    static void ExecuteConsoleCommand(UWorld* World, const FString& Command);

    // Cached settings
    static FS_GameplaySettings CurrentSettings;
    static bool bIsInitialized;
};