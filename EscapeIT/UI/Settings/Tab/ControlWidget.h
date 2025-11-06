// ControlWidget.h (final)
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "ControlWidget.generated.h"

class USelectionSettingRow;
class UButton;
class UNumericSettingRow;

UCLASS()
class ESCAPEIT_API UControlWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UControlWidget(const FObjectInitializer& ObjectInitializer);

    // UUserWidget
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Public API
    void LoadSettings(const FS_ControlSettings& Settings);
    FS_ControlSettings GetCurrentSettings() const;
    TArray<FString> ValidateSettings() const;

protected:
    // Selection rows (reusable SelectionSettingRow)
    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* InvertMouseYRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* InvertGamepadYRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* GamepadVibrationRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* AutoSprintRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* CrouchToggleRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* FlashlightToggleRow;

    // Reusable numeric rows (Bind instances of your NumericSettingRow UMG)
    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* MouseSensitivityRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* CameraZoomSensitivityRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* GamepadSensitivityRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* GamepadDeadzoneRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* GamepadVibrationIntensityRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* RebindKeysButton;

    // Internal
    UPROPERTY()
    class USettingsSubsystem* SettingsSubsystem;

    // Flags
    bool bIsLoadingSettings;

    // Current local settings (widget-level, not yet saved to subsystem)
    FS_ControlSettings CurrentSettings;

    // Initialization
    void InitializeSelectionRows();

    // Helpers
    TArray<FText> MakeToggleOptions() const;
    TArray<FText> MakeHoldToggleOptions() const;

    // Selection callbacks
    UFUNCTION()
    void OnInvertMouseYChanged(int32 NewIndex);

    UFUNCTION()
    void OnInvertGamepadYChanged(int32 NewIndex);

    UFUNCTION()
    void OnGamepadVibrationChanged(int32 NewIndex);

    UFUNCTION()
    void OnAutoSprintChanged(int32 NewIndex);

    UFUNCTION()
    void OnCrouchToggleChanged(int32 NewIndex);

    UFUNCTION()
    void OnFlashlightToggleChanged(int32 NewIndex);

    // Button
    UFUNCTION()
    void OnRebindKeysButtonClicked();

    // Numeric row handlers
    UFUNCTION()
    void HandleMouseSensitivityRowChanged(float NewValue);

    UFUNCTION()
    void HandleCameraZoomSensitivityRowChanged(float NewValue);

    UFUNCTION()
    void HandleGamepadSensitivityRowChanged(float NewValue);

    UFUNCTION()
    void HandleGamepadDeadzoneRowChanged(float NewValue);

    UFUNCTION()
    void HandleGamepadVibrationIntensityRowChanged(float NewValue);
};