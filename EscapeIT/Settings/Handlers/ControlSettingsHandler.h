#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Data/SettingsTypes.h"

class ESCAPEIT_API FControlSettingsHandler
{
public:
    // Apply control settings to engine / current world
    static void ApplyToEngine(const FS_ControlSettings& Settings, UWorld* World);

    // Individual setters
    static void SetMouseSensitivity(float Sensitivity, UWorld* World);
    static void SetInvertMouseY(bool bInvert, UWorld* World);
    static void SetCameraZoomSensitivity(float Sensitivity, UWorld* World);

    static void SetGamepadSensitivity(float Sensitivity, UWorld* World);
    static void SetGamepadDeadzone(float Deadzone, UWorld* World);
    static void SetGamepadVibrationEnabled(bool bEnabled, UWorld* World);
    static void SetGamepadVibrationIntensity(float Intensity, UWorld* World);

    static void SetAutoSprintEnabled(bool bEnabled);
    static void SetCrouchToggle(bool bToggle);
    static void SetFlashlightToggle(bool bToggle);

private:
    // Helpers
    static void ExecuteConsoleCommand(UWorld* World, const FString& Command);
    static void ApplyToAllPlayerControllers(UWorld* World, TFunctionRef<void(APlayerController*)> Func);
    static void SaveToConfig(const FS_ControlSettings& Settings);
};
