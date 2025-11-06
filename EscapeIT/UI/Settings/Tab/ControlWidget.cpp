
#include "ControlWidget.h"
#include "EscapeIT/UI/Settings/Row/NumericSettingRow.h"
#include "EscapeIT/UI/Settings/Row/SelectionSettingRow.h"
#include "EscapeIT/Settings/Core/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

UControlWidget::UControlWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsLoadingSettings(false)
    , SettingsSubsystem(nullptr)
{
}

void UControlWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get Settings Subsystem (optional)
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
    }

    if (!SettingsSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("ControlWidget: SettingsSubsystem not found; widget will still work locally"));
    }

    // Initialize selection rows and bind their delegates
    InitializeSelectionRows();

    // Initialize numeric rows (bind delegates)
    if (MouseSensitivityRow)
    {
        MouseSensitivityRow->InitializeRow(0.1f, 3.0f, 0.05f, CurrentSettings.MouseSensitivity, FText::FromString(TEXT("Mouse Sensitivity")));
        MouseSensitivityRow->OnNumericValueChanged.AddDynamic(this, &UControlWidget::HandleMouseSensitivityRowChanged);
    }

    if (CameraZoomSensitivityRow)
    {
        CameraZoomSensitivityRow->InitializeRow(0.0f, 1.0f, 0.05f, CurrentSettings.CameraZoomSensitivity, FText::FromString(TEXT("Camera Zoom")));
        CameraZoomSensitivityRow->OnNumericValueChanged.AddDynamic(this, &UControlWidget::HandleCameraZoomSensitivityRowChanged);
    }

    if (GamepadSensitivityRow)
    {
        GamepadSensitivityRow->InitializeRow(0.1f, 3.0f, 0.05f, CurrentSettings.GamepadSensitivity, FText::FromString(TEXT("Gamepad Sensitivity")));
        GamepadSensitivityRow->OnNumericValueChanged.AddDynamic(this, &UControlWidget::HandleGamepadSensitivityRowChanged);
    }

    if (GamepadDeadzoneRow)
    {
        GamepadDeadzoneRow->InitializeRow(0.0f, 0.5f, 0.01f, CurrentSettings.GamepadDeadzone, FText::FromString(TEXT("Gamepad Deadzone")));
        GamepadDeadzoneRow->OnNumericValueChanged.AddDynamic(this, &UControlWidget::HandleGamepadDeadzoneRowChanged);
    }

    if (GamepadVibrationIntensityRow)
    {
        GamepadVibrationIntensityRow->InitializeRow(0.0f, 1.0f, 0.05f, CurrentSettings.GamepadVibrationIntensity, FText::FromString(TEXT("Gamepad Vibration")));
        GamepadVibrationIntensityRow->OnNumericValueChanged.AddDynamic(this, &UControlWidget::HandleGamepadVibrationIntensityRowChanged);
    }

    // Load current settings either from subsystem (if available) or defaults
    if (SettingsSubsystem)
    {
        LoadSettings(SettingsSubsystem->GetAllSettings().ControlSettings);
    }
    else
    {
        FS_ControlSettings DefaultSettings;
        LoadSettings(DefaultSettings);
    }

    // Bind button
    if (RebindKeysButton)
    {
        RebindKeysButton->OnClicked.AddDynamic(this, &UControlWidget::OnRebindKeysButtonClicked);
    }
}

void UControlWidget::NativeDestruct()
{
    // Unbind selection row delegates
    if (InvertMouseYRow)
        InvertMouseYRow->OnSelectionChanged.RemoveDynamic(this, &UControlWidget::OnInvertMouseYChanged);
    if (InvertGamepadYRow)
        InvertGamepadYRow->OnSelectionChanged.RemoveDynamic(this, &UControlWidget::OnInvertGamepadYChanged);
    if (GamepadVibrationRow)
        GamepadVibrationRow->OnSelectionChanged.RemoveDynamic(this, &UControlWidget::OnGamepadVibrationChanged);
    if (AutoSprintRow)
        AutoSprintRow->OnSelectionChanged.RemoveDynamic(this, &UControlWidget::OnAutoSprintChanged);
    if (CrouchToggleRow)
        CrouchToggleRow->OnSelectionChanged.RemoveDynamic(this, &UControlWidget::OnCrouchToggleChanged);
    if (FlashlightToggleRow)
        FlashlightToggleRow->OnSelectionChanged.RemoveDynamic(this, &UControlWidget::OnFlashlightToggleChanged);

    // Unbind numeric row delegates
    if (MouseSensitivityRow)
        MouseSensitivityRow->OnNumericValueChanged.RemoveDynamic(this, &UControlWidget::HandleMouseSensitivityRowChanged);
    if (CameraZoomSensitivityRow)
        CameraZoomSensitivityRow->OnNumericValueChanged.RemoveDynamic(this, &UControlWidget::HandleCameraZoomSensitivityRowChanged);
    if (GamepadSensitivityRow)
        GamepadSensitivityRow->OnNumericValueChanged.RemoveDynamic(this, &UControlWidget::HandleGamepadSensitivityRowChanged);
    if (GamepadDeadzoneRow)
        GamepadDeadzoneRow->OnNumericValueChanged.RemoveDynamic(this, &UControlWidget::HandleGamepadDeadzoneRowChanged);
    if (GamepadVibrationIntensityRow)
        GamepadVibrationIntensityRow->OnNumericValueChanged.RemoveDynamic(this, &UControlWidget::HandleGamepadVibrationIntensityRowChanged);

    // Unbind button
    if (RebindKeysButton)
        RebindKeysButton->OnClicked.RemoveDynamic(this, &UControlWidget::OnRebindKeysButtonClicked);

    Super::NativeDestruct();
}

void UControlWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

// Initialize selection rows with options and bind delegates
void UControlWidget::InitializeSelectionRows()
{
    const TArray<FText> ToggleOptions = MakeToggleOptions();
    const TArray<FText> HoldToggleOptions = MakeHoldToggleOptions();

    if (InvertMouseYRow)
    {
        InvertMouseYRow->InitializeRow(ToggleOptions, CurrentSettings.bInvertMouseY ? 1 : 0, FText::FromString(TEXT("Invert Mouse Y")));
        InvertMouseYRow->OnSelectionChanged.AddDynamic(this, &UControlWidget::OnInvertMouseYChanged);
    }

    if (InvertGamepadYRow)
    {
        InvertGamepadYRow->InitializeRow(ToggleOptions, CurrentSettings.bInvertGamepadY ? 1 : 0, FText::FromString(TEXT("Invert Gamepad Y")));
        InvertGamepadYRow->OnSelectionChanged.AddDynamic(this, &UControlWidget::OnInvertGamepadYChanged);
    }

    if (GamepadVibrationRow)
    {
        GamepadVibrationRow->InitializeRow(ToggleOptions, CurrentSettings.bGamepadVibrationEnabled ? 1 : 0, FText::FromString(TEXT("Gamepad Vibration")));
        GamepadVibrationRow->OnSelectionChanged.AddDynamic(this, &UControlWidget::OnGamepadVibrationChanged);
    }

    if (AutoSprintRow)
    {
        AutoSprintRow->InitializeRow(ToggleOptions, CurrentSettings.bAutoSprintEnabled ? 1 : 0, FText::FromString(TEXT("Auto Sprint")));
        AutoSprintRow->OnSelectionChanged.AddDynamic(this, &UControlWidget::OnAutoSprintChanged);
    }

    if (CrouchToggleRow)
    {
        CrouchToggleRow->InitializeRow(HoldToggleOptions, CurrentSettings.bCrouchToggle ? 1 : 0, FText::FromString(TEXT("Crouch")));
        CrouchToggleRow->OnSelectionChanged.AddDynamic(this, &UControlWidget::OnCrouchToggleChanged);
    }

    if (FlashlightToggleRow)
    {
        FlashlightToggleRow->InitializeRow(HoldToggleOptions, CurrentSettings.bFlashlightToggle ? 1 : 0, FText::FromString(TEXT("Flashlight")));
        FlashlightToggleRow->OnSelectionChanged.AddDynamic(this, &UControlWidget::OnFlashlightToggleChanged);
    }
}

TArray<FText> UControlWidget::MakeToggleOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Off")));
    Out.Add(FText::FromString(TEXT("On")));
    return Out;
}

TArray<FText> UControlWidget::MakeHoldToggleOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Hold")));
    Out.Add(FText::FromString(TEXT("Toggle")));
    return Out;
}

void UControlWidget::LoadSettings(const FS_ControlSettings& Settings)
{
    bIsLoadingSettings = true;
    CurrentSettings = Settings;

    // Update numeric rows (do not trigger delegates)
    if (MouseSensitivityRow)
        MouseSensitivityRow->SetValue(CurrentSettings.MouseSensitivity, false);

    if (CameraZoomSensitivityRow)
        CameraZoomSensitivityRow->SetValue(CurrentSettings.CameraZoomSensitivity, false);

    if (GamepadSensitivityRow)
        GamepadSensitivityRow->SetValue(CurrentSettings.GamepadSensitivity, false);

    if (GamepadDeadzoneRow)
        GamepadDeadzoneRow->SetValue(CurrentSettings.GamepadDeadzone, false);

    if (GamepadVibrationIntensityRow)
        GamepadVibrationIntensityRow->SetValue(CurrentSettings.GamepadVibrationIntensity, false);

    // Update selection rows (do not trigger delegates)
    if (InvertMouseYRow)
        InvertMouseYRow->SetCurrentSelection(CurrentSettings.bInvertMouseY ? 1 : 0, false);

    if (InvertGamepadYRow)
        InvertGamepadYRow->SetCurrentSelection(CurrentSettings.bInvertGamepadY ? 1 : 0, false);

    if (GamepadVibrationRow)
        GamepadVibrationRow->SetCurrentSelection(CurrentSettings.bGamepadVibrationEnabled ? 1 : 0, false);

    if (AutoSprintRow)
        AutoSprintRow->SetCurrentSelection(CurrentSettings.bAutoSprintEnabled ? 1 : 0, false);

    if (CrouchToggleRow)
        CrouchToggleRow->SetCurrentSelection(CurrentSettings.bCrouchToggle ? 1 : 0, false);

    if (FlashlightToggleRow)
        FlashlightToggleRow->SetCurrentSelection(CurrentSettings.bFlashlightToggle ? 1 : 0, false);

    bIsLoadingSettings = false;
}

FS_ControlSettings UControlWidget::GetCurrentSettings() const
{
    return CurrentSettings;
}

TArray<FString> UControlWidget::ValidateSettings() const
{
    TArray<FString> Errors;
    if (CurrentSettings.MouseSensitivity < 0.1f || CurrentSettings.MouseSensitivity > 3.0f)
    {
        Errors.Add(TEXT("Mouse sensitivity out of range (0.1 - 3.0)"));
    }
    if (CurrentSettings.GamepadDeadzone < 0.0f || CurrentSettings.GamepadDeadzone > 0.5f)
    {
        Errors.Add(TEXT("Gamepad deadzone out of range (0.0 - 0.5)"));
    }
    if (Errors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ControlWidget: Validation found %d errors"), Errors.Num());
    }
    return Errors;
}

// Selection callbacks

void UControlWidget::OnInvertMouseYChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bInvertMouseY = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: InvertMouseY changed to %s (local)"), CurrentSettings.bInvertMouseY ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::OnInvertGamepadYChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bInvertGamepadY = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: InvertGamepadY changed to %s (local)"), CurrentSettings.bInvertGamepadY ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::OnGamepadVibrationChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bGamepadVibrationEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: GamepadVibration changed to %s (local)"), CurrentSettings.bGamepadVibrationEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::OnAutoSprintChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bAutoSprintEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: AutoSprint changed to %s (local)"), CurrentSettings.bAutoSprintEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::OnCrouchToggleChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bCrouchToggle = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: CrouchToggle changed to %s (local)"), CurrentSettings.bCrouchToggle ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::OnFlashlightToggleChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bFlashlightToggle = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: FlashlightToggle changed to %s (local)"), CurrentSettings.bFlashlightToggle ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

// Numeric row handlers

void UControlWidget::HandleMouseSensitivityRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.MouseSensitivity = NewValue;
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: MouseSensitivity updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::HandleCameraZoomSensitivityRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.CameraZoomSensitivity = NewValue;
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: CameraZoomSensitivity updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::HandleGamepadSensitivityRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.GamepadSensitivity = NewValue;
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: GamepadSensitivity updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::HandleGamepadDeadzoneRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.GamepadDeadzone = NewValue;
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: GamepadDeadzone updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::HandleGamepadVibrationIntensityRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.GamepadVibrationIntensity = NewValue;
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: GamepadVibrationIntensity updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyControlSettings(CurrentSettings);
}

void UControlWidget::OnRebindKeysButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("ControlWidget: Rebind Keys clicked - open key binding UI (not implemented here)"));
}
