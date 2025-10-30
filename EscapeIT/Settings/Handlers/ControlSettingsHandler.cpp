#include "EscapeIT/Settings/Handlers/ControlSettingsHandler.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "EscapeIT/EscapeITPlayerController.h"

// Config section used to persist control settings (can be read by your systems later)
static const TCHAR* ControlConfigSection = TEXT("/Script/EscapeIT.ControlSettings");

// ===== MAIN APPLY FUNCTION =====
void FControlSettingsHandler::ApplyToEngine(const FS_ControlSettings& Settings, UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ControlSettingsHandler: World is null - cannot apply controls"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Applying control settings..."));

    SetMouseSensitivity(Settings.MouseSensitivity, World);
    SetInvertMouseY(Settings.bInvertMouseY, World);
    SetCameraZoomSensitivity(Settings.CameraZoomSensitivity, World);

    SetGamepadSensitivity(Settings.GamepadSensitivity, World);
    SetGamepadDeadzone(Settings.GamepadDeadzone, World);

    SetGamepadVibrationEnabled(Settings.bGamepadVibrationEnabled, World);
    SetGamepadVibrationIntensity(Settings.GamepadVibrationIntensity, World);

    SetAutoSprintEnabled(Settings.bAutoSprintEnabled);
    SetCrouchToggle(Settings.bCrouchToggle);
    SetFlashlightToggle(Settings.bFlashlightToggle);

    // Persist settings for other subsystems / next session
    SaveToConfig(Settings);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Control settings applied"));
}

// ===== INDIVIDUAL SETTERS =====

void FControlSettingsHandler::SetMouseSensitivity(float Sensitivity, UWorld* World)
{
    Sensitivity = FMath::Clamp(Sensitivity, 0.01f, 10.0f);

    ApplyToAllPlayerControllers(World, [Sensitivity](APlayerController* BasePC)
        {
            if (!BasePC) return;
            if (AEscapeITPlayerController* PC = Cast<AEscapeITPlayerController>(BasePC))
            {
                PC->SetMouseSensitivity(Sensitivity);
            }
        });

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Mouse sensitivity set to %.3f"), Sensitivity);
}

void FControlSettingsHandler::SetInvertMouseY(bool bInvert, UWorld* World)
{
    ApplyToAllPlayerControllers(World, [bInvert](APlayerController* BasePC)
        {
            if (!BasePC) return;
            if (AEscapeITPlayerController* PC = Cast<AEscapeITPlayerController>(BasePC))
            {
                PC->SetInvertPitch(bInvert);
            }
        });


    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Invert mouse Y %s"), bInvert ? TEXT("enabled") : TEXT("disabled"));
}

void FControlSettingsHandler::SetCameraZoomSensitivity(float Sensitivity, UWorld* World)
{
    // Camera zoom sensitivity is usually game-specific — we expose it to config and a console command so gameplay code can pick it up.
    Sensitivity = FMath::Clamp(Sensitivity, 0.0f, 10.0f);

    // Save to config for gameplay systems to read
    GConfig->SetFloat(ControlConfigSection, TEXT("CameraZoomSensitivity"), Sensitivity, GGameIni);
    GConfig->Flush(false, GGameIni);

    // Also expose via console variable (optional)
    FString Cmd = FString::Printf(TEXT("set Camera.ZoomSensitivity %.3f"), Sensitivity);
    ExecuteConsoleCommand(World, Cmd);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Camera zoom sensitivity set to %.3f"), Sensitivity);
}

void FControlSettingsHandler::SetGamepadSensitivity(float Sensitivity, UWorld* World)
{
    Sensitivity = FMath::Clamp(Sensitivity, 0.01f, 10.0f);

    // Nếu bạn đã tạo AMyPlayerController với SetGamepadSensitivity, gọi nó.
    ApplyToAllPlayerControllers(World, [Sensitivity](APlayerController* BasePC)
        {
            if (!BasePC) return;

            // Nếu dùng PlayerController tuỳ chỉnh:
            if (AEscapeITPlayerController* PC = Cast<AEscapeITPlayerController>(BasePC))
            {
                PC->SetGamepadSensitivity(Sensitivity);
            }
            else
            {
                // Fallback: lưu vào một console command / cvar để hệ thống input tự lấy (tuỳ project)
                // Ví dụ expose qua console var:
                FString Cmd = FString::Printf(TEXT("set Gamepad.Sensitivity %.3f"), Sensitivity);
                if (GEngine && BasePC->GetWorld())
                {
                    GEngine->Exec(BasePC->GetWorld(), *Cmd);
                }
                // Hoặc đẩy vào một singleton / Pawn property mà input handler đọc.
            }
        });

    // Persist
    GConfig->SetFloat(ControlConfigSection, TEXT("GamepadSensitivity"), Sensitivity, GGameIni);
    GConfig->Flush(false, GGameIni);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Gamepad sensitivity set to %.3f"), Sensitivity);
}


void FControlSettingsHandler::SetGamepadDeadzone(float Deadzone, UWorld* World)
{
    Deadzone = FMath::Clamp(Deadzone, 0.0f, 1.0f);

    // Many engines handle deadzone inside input mapping. Persist value so that input system can read it.
    GConfig->SetFloat(ControlConfigSection, TEXT("GamepadDeadzone"), Deadzone, GGameIni);
    GConfig->Flush(false, GGameIni);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Gamepad deadzone set to %.3f"), Deadzone);
}

void FControlSettingsHandler::SetGamepadVibrationEnabled(bool bEnabled, UWorld* World)
{
    GConfig->SetBool(ControlConfigSection, TEXT("GamepadVibrationEnabled"), bEnabled, GGameIni);
    GConfig->Flush(false, GGameIni);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Gamepad vibration %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void FControlSettingsHandler::SetGamepadVibrationIntensity(float Intensity, UWorld* World)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    GConfig->SetFloat(ControlConfigSection, TEXT("GamepadVibrationIntensity"), Intensity, GGameIni);
    GConfig->Flush(false, GGameIni);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Gamepad vibration intensity set to %.3f"), Intensity);
}

void FControlSettingsHandler::SetAutoSprintEnabled(bool bEnabled)
{
    // Persist toggle for gameplay systems to read
    GConfig->SetBool(ControlConfigSection, TEXT("AutoSprintEnabled"), bEnabled, GGameIni);
    GConfig->Flush(false, GGameIni);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Auto-sprint %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void FControlSettingsHandler::SetCrouchToggle(bool bToggle)
{
    GConfig->SetBool(ControlConfigSection, TEXT("CrouchToggle"), bToggle, GGameIni);
    GConfig->Flush(false, GGameIni);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Crouch toggle %s"), bToggle ? TEXT("enabled") : TEXT("disabled"));
}

void FControlSettingsHandler::SetFlashlightToggle(bool bToggle)
{
    GConfig->SetBool(ControlConfigSection, TEXT("FlashlightToggle"), bToggle, GGameIni);
    GConfig->Flush(false, GGameIni);

    UE_LOG(LogTemp, Log, TEXT("ControlSettingsHandler: Flashlight toggle %s"), bToggle ? TEXT("enabled") : TEXT("disabled"));
}

// ===== HELPERS =====

void FControlSettingsHandler::ExecuteConsoleCommand(UWorld* World, const FString& Command)
{
    if (!World || !GEngine)
    {
        UE_LOG(LogTemp, Warning, TEXT("ControlSettingsHandler: Cannot execute command '%s' - World or Engine is null"), *Command);
        return;
    }

    GEngine->Exec(World, *Command);
}

void FControlSettingsHandler::ApplyToAllPlayerControllers(UWorld* World, TFunctionRef<void(APlayerController*)> Func)
{
    if (!World) return;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC)
        {
            Func(PC);
        }
    }
}

void FControlSettingsHandler::SaveToConfig(const FS_ControlSettings& Settings)
{
    GConfig->SetFloat(ControlConfigSection, TEXT("MouseSensitivity"), Settings.MouseSensitivity, GGameIni);
    GConfig->SetBool(ControlConfigSection, TEXT("InvertMouseY"), Settings.bInvertMouseY, GGameIni);
    GConfig->SetFloat(ControlConfigSection, TEXT("CameraZoomSensitivity"), Settings.CameraZoomSensitivity, GGameIni);

    GConfig->SetFloat(ControlConfigSection, TEXT("GamepadSensitivity"), Settings.GamepadSensitivity, GGameIni);
    GConfig->SetFloat(ControlConfigSection, TEXT("GamepadDeadzone"), Settings.GamepadDeadzone, GGameIni);
    GConfig->SetBool(ControlConfigSection, TEXT("GamepadVibrationEnabled"), Settings.bGamepadVibrationEnabled, GGameIni);
    GConfig->SetFloat(ControlConfigSection, TEXT("GamepadVibrationIntensity"), Settings.GamepadVibrationIntensity, GGameIni);

    GConfig->SetBool(ControlConfigSection, TEXT("AutoSprintEnabled"), Settings.bAutoSprintEnabled, GGameIni);
    GConfig->SetBool(ControlConfigSection, TEXT("CrouchToggle"), Settings.bCrouchToggle, GGameIni);
    GConfig->SetBool(ControlConfigSection, TEXT("FlashlightToggle"), Settings.bFlashlightToggle, GGameIni);

    GConfig->Flush(false, GGameIni);
}
