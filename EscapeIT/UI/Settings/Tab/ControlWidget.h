// ControlWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "ControlWidget.generated.h"

class USelectionWidget;
class USlider;
class UTextBlock;
class UButton;

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
	// UI refs (bind in UMG)
	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* InvertMouseYSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* InvertGamepadYSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* GamepadVibrationSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* AutoSprintSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* CrouchToggleSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* FlashlightToggleSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* MouseSensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MouseSensitivityText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* CameraZoomSensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CameraZoomSensitivityText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* GamepadSensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* GamepadSensitivityText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* GamepadDeadzoneSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* GamepadDeadzoneText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* GamepadVibrationIntensitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* GamepadVibrationIntensityText;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* RebindKeysButton;

	// Internal
	UPROPERTY()
	class USettingsSubsystem* SettingsSubsystem;

	// Flags to avoid feedback loops
	bool bUpdatingSliders;
	bool bIsLoadingSettings;

	// Current local settings (widget-level, not yet saved to subsystem)
	FS_ControlSettings CurrentSettings;

	// Initialization
	void InitializeSelections();
	void InitializeSliders();
	void BindSliderEvents();
	void UnbindSliderEvents();

	// Helpers
	void AddToggleOptions(USelectionWidget* Selection);
	void AddHoldToggleOptions(USelectionWidget* Selection);
	void UpdateVolumeText();

	// Slider callbacks (now update CurrentSettings only)
	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);

	UFUNCTION()
	void OnCameraZoomSensitivityChanged(float Value);

	UFUNCTION()
	void OnGamepadSensitivityChanged(float Value);

	UFUNCTION()
	void OnGamepadDeadzoneChanged(float Value);

	UFUNCTION()
	void OnGamepadVibrationIntensityChanged(float Value);

	// Selection callbacks (update CurrentSettings only)
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

	// Buttons
	UFUNCTION()
	void OnRebindKeysButtonClicked();
};
