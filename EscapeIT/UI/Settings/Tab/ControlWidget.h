// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ControlWidget.generated.h"

class USelectionWidget;
class USettingsSubsystem;
class UButton;
class USlider;
class UTextBlock;

/**
 * Widget for Control Settings
 */
UCLASS()
class ESCAPEIT_API UControlWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UControlWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ===== WIDGET BINDINGS =====

	// === MOUSE SETTINGS ===
	UPROPERTY(meta = (BindWidget))
	USlider* MouseSensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MouseSensitivityText;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* InvertMouseYSelection;

	UPROPERTY(meta = (BindWidget))
	USlider* CameraZoomSensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CameraZoomSensitivityText;

	// === GAMEPAD SETTINGS ===

	UPROPERTY(meta = (BindWidget))
	USlider* GamepadSensitivitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* GamepadSensitivityText;

	UPROPERTY(meta = (BindWidget))
	USlider* GamepadDeadzoneSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* GamepadDeadzoneText;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* InvertGamepadYSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* GamepadVibrationSelection;

	UPROPERTY(meta = (BindWidget))
	USlider* GamepadVibrationIntensitySlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* GamepadVibrationIntensityText;

	// === GAMEPLAY CONTROLS ===

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* AutoSprintSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* CrouchToggleSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* FlashlightToggleSelection;

	// === KEY BINDINGS (Optional) ===

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* RebindKeysButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetButton;

	// ===== CALLBACKS =====

	// Mouse callbacks
	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);

	UFUNCTION()
	void OnInvertMouseYChanged(int32 NewIndex);

	UFUNCTION()
	void OnCameraZoomSensitivityChanged(float Value);

	// Gamepad callbacks
	UFUNCTION()
	void OnGamepadSensitivityChanged(float Value);

	UFUNCTION()
	void OnGamepadDeadzoneChanged(float Value);

	UFUNCTION()
	void OnInvertGamepadYChanged(int32 NewIndex);

	UFUNCTION()
	void OnGamepadVibrationChanged(int32 NewIndex);

	UFUNCTION()
	void OnGamepadVibrationIntensityChanged(float Value);

	// Gameplay controls callbacks
	UFUNCTION()
	void OnAutoSprintChanged(int32 NewIndex);

	UFUNCTION()
	void OnCrouchToggleChanged(int32 NewIndex);

	UFUNCTION()
	void OnFlashlightToggleChanged(int32 NewIndex);

	// Button callbacks
	UFUNCTION()
	void OnRebindKeysButtonClicked();

	UFUNCTION()
	void OnResetButtonClicked();

private:
	/** Reference to Settings Subsystem */
	UPROPERTY()
	USettingsSubsystem* SettingsSubsystem;

	bool bUpdatingSliders;

	void InitializeSelections();
	void InitializeSliders();
	void LoadCurrentSettings();
	void AddToggleOptions(USelectionWidget* Selection);
	void AddHoldToggleOptions(USelectionWidget* Selection);
	void UpdateVolumeText();
	void BindSliderEvents();
	void UnbindSliderEvents();
};