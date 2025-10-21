// Fill out your copyright notice in the Description page of Project Settings.

#include "ControlWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Subsystem/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"

UControlWidget::UControlWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUpdatingSliders(false)
{
}

void UControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get Settings Subsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
	}

	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("ControlWidget: Failed to get SettingsSubsystem"));
		return;
	}

	// Initialize all selections
	InitializeSelections();

	// Initialize sliders
	InitializeSliders();

	// Bind slider events
	BindSliderEvents();

	// Load current settings
	LoadCurrentSettings();

	// Bind buttons
	if (RebindKeysButton)
	{
		RebindKeysButton->OnClicked.AddDynamic(this, &UControlWidget::OnRebindKeysButtonClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UControlWidget::OnResetButtonClicked);
	}
}

void UControlWidget::NativeDestruct()
{
	// Unbind all events
	UnbindSliderEvents();

	if (RebindKeysButton)
	{
		RebindKeysButton->OnClicked.RemoveDynamic(this, &UControlWidget::OnRebindKeysButtonClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UControlWidget::OnResetButtonClicked);
	}

	Super::NativeDestruct();
}

void UControlWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update progress bars if they exist
	if (!bUpdatingSliders)
	{
		UpdateProgressBars();
	}
}

void UControlWidget::InitializeSelections()
{
	// Invert Mouse Y
	if (InvertMouseYSelection)
	{
		InvertMouseYSelection->Clear();
		AddToggleOptions(InvertMouseYSelection);
		InvertMouseYSelection->OnSelectionChanged.BindDynamic(this, &UControlWidget::OnInvertMouseYChanged);
	}

	// Invert Gamepad Y
	if (InvertGamepadYSelection)
	{
		InvertGamepadYSelection->Clear();
		AddToggleOptions(InvertGamepadYSelection);
		InvertGamepadYSelection->OnSelectionChanged.BindDynamic(this, &UControlWidget::OnInvertGamepadYChanged);
	}

	// Gamepad Vibration
	if (GamepadVibrationSelection)
	{
		GamepadVibrationSelection->Clear();
		AddToggleOptions(GamepadVibrationSelection);
		GamepadVibrationSelection->OnSelectionChanged.BindDynamic(this, &UControlWidget::OnGamepadVibrationChanged);
	}

	// Auto Sprint
	if (AutoSprintSelection)
	{
		AutoSprintSelection->Clear();
		AddToggleOptions(AutoSprintSelection);
		AutoSprintSelection->OnSelectionChanged.BindDynamic(this, &UControlWidget::OnAutoSprintChanged);
	}

	// Crouch Toggle
	if (CrouchToggleSelection)
	{
		CrouchToggleSelection->Clear();
		AddHoldToggleOptions(CrouchToggleSelection);
		CrouchToggleSelection->OnSelectionChanged.BindDynamic(this, &UControlWidget::OnCrouchToggleChanged);
	}

	// Flashlight Toggle
	if (FlashlightToggleSelection)
	{
		FlashlightToggleSelection->Clear();
		AddHoldToggleOptions(FlashlightToggleSelection);
		FlashlightToggleSelection->OnSelectionChanged.BindDynamic(this, &UControlWidget::OnFlashlightToggleChanged);
	}
}

void UControlWidget::InitializeSliders()
{
	// Mouse Sensitivity (0.1 to 3.0)
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->SetMinValue(0.1f);
		MouseSensitivitySlider->SetMaxValue(3.0f);
		MouseSensitivitySlider->SetStepSize(0.05f);
	}

	// Camera Zoom Sensitivity (0.0 to 1.0)
	if (CameraZoomSensitivitySlider)
	{
		CameraZoomSensitivitySlider->SetMinValue(0.0f);
		CameraZoomSensitivitySlider->SetMaxValue(1.0f);
		CameraZoomSensitivitySlider->SetStepSize(0.05f);
	}

	// Gamepad Sensitivity (0.1 to 3.0)
	if (GamepadSensitivitySlider)
	{
		GamepadSensitivitySlider->SetMinValue(0.1f);
		GamepadSensitivitySlider->SetMaxValue(3.0f);
		GamepadSensitivitySlider->SetStepSize(0.05f);
	}

	// Gamepad Deadzone (0.0 to 0.5)
	if (GamepadDeadzoneSlider)
	{
		GamepadDeadzoneSlider->SetMinValue(0.0f);
		GamepadDeadzoneSlider->SetMaxValue(0.5f);
		GamepadDeadzoneSlider->SetStepSize(0.01f);
	}

	// Gamepad Vibration Intensity (0.0 to 1.0)
	if (GamepadVibrationIntensitySlider)
	{
		GamepadVibrationIntensitySlider->SetMinValue(0.0f);
		GamepadVibrationIntensitySlider->SetMaxValue(1.0f);
		GamepadVibrationIntensitySlider->SetStepSize(0.05f);
	}
}

void UControlWidget::LoadCurrentSettings()
{
	if (!SettingsSubsystem)
		return;

	bUpdatingSliders = true; // Prevent feedback loop

	FS_ControlSettings CurrentSettings = SettingsSubsystem->GetAllSettings().ControlSettings;

	// Mouse Settings
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->SetValue(CurrentSettings.MouseSensitivity);
	}

	if (InvertMouseYSelection)
	{
		InvertMouseYSelection->SetCurrentSelection(CurrentSettings.bInvertMouseY ? 1 : 0);
	}

	if (CameraZoomSensitivitySlider)
	{
		CameraZoomSensitivitySlider->SetValue(CurrentSettings.CameraZoomSensitivity);
	}

	// Gamepad Settings
	if (GamepadSensitivitySlider)
	{
		GamepadSensitivitySlider->SetValue(CurrentSettings.GamepadSensitivity);
	}

	if (GamepadDeadzoneSlider)
	{
		GamepadDeadzoneSlider->SetValue(CurrentSettings.GamepadDeadzone);
	}

	if (InvertGamepadYSelection)
	{
		InvertGamepadYSelection->SetCurrentSelection(CurrentSettings.bInvertGamepadY ? 1 : 0);
	}

	if (GamepadVibrationSelection)
	{
		GamepadVibrationSelection->SetCurrentSelection(CurrentSettings.bGamepadVibrationEnabled ? 1 : 0);
	}

	if (GamepadVibrationIntensitySlider)
	{
		GamepadVibrationIntensitySlider->SetValue(CurrentSettings.GamepadVibrationIntensity);
	}

	// Gameplay Controls
	if (AutoSprintSelection)
	{
		AutoSprintSelection->SetCurrentSelection(CurrentSettings.bAutoSprintEnabled ? 1 : 0);
	}

	if (CrouchToggleSelection)
	{
		CrouchToggleSelection->SetCurrentSelection(CurrentSettings.bCrouchToggle ? 1 : 0);
	}

	if (FlashlightToggleSelection)
	{
		FlashlightToggleSelection->SetCurrentSelection(CurrentSettings.bFlashlightToggle ? 1 : 0);
	}

	bUpdatingSliders = false;
	UpdateProgressBars();
}

// ===== MOUSE CALLBACKS =====

void UControlWidget::OnMouseSensitivityChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetMouseSensitivity(Value);
	}
}

void UControlWidget::OnInvertMouseYChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetInvertMouseY(NewIndex == 1);
	}
}

void UControlWidget::OnCameraZoomSensitivityChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetCameraZoomSensitivity(Value);
	}
}

// ===== GAMEPAD CALLBACKS =====

void UControlWidget::OnGamepadSensitivityChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetGamepadSensitivity(Value);
	}
}

void UControlWidget::OnGamepadDeadzoneChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetGamepadDeadzone(Value);
	}
}

void UControlWidget::OnInvertGamepadYChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetInvertGamepadY(NewIndex == 1);
	}
}

void UControlWidget::OnGamepadVibrationChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetGamepadVibrationEnabled(NewIndex == 1);
	}
}

void UControlWidget::OnGamepadVibrationIntensityChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetGamepadVibrationIntensity(Value);
	}
}

// ===== GAMEPLAY CONTROLS CALLBACKS =====

void UControlWidget::OnAutoSprintChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetAutoSprintEnabled(NewIndex == 1);
	}
}

void UControlWidget::OnCrouchToggleChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetCrouchToggle(NewIndex == 1);
	}
}

void UControlWidget::OnFlashlightToggleChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetFlashlightToggle(NewIndex == 1);
	}
}

// ===== BUTTON CALLBACKS =====

void UControlWidget::OnRebindKeysButtonClicked()
{
	// TODO: Open key binding UI
	// This would typically open another widget for rebinding keys
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: Rebind Keys clicked - Implement key binding UI"));

	// Example: You could broadcast an event or open a new widget
	// OnOpenKeyBindingWidget.Broadcast();
}

void UControlWidget::OnResetButtonClicked()
{
	if (SettingsSubsystem)
	{
		// Reset only control settings to default
		FS_ControlSettings DefaultSettings;
		SettingsSubsystem->ApplyControlSettings(DefaultSettings);

		// Reload UI
		LoadCurrentSettings();
	}
}

// ===== HELPER FUNCTIONS =====

void UControlWidget::AddToggleOptions(USelectionWidget* Selection)
{
	if (Selection)
	{
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("Off")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("On")) });
	}
}

void UControlWidget::AddHoldToggleOptions(USelectionWidget* Selection)
{
	if (Selection)
	{
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("Hold")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("Toggle")) });
	}
}

void UControlWidget::UpdateProgressBars()
{
	// Update progress bars to match slider values

	// Mouse sensitivity (normalize 0.1-3.0 to 0.0-1.0)
	if (MouseSensitivityBar && MouseSensitivitySlider)
	{
		float NormalizedValue = (MouseSensitivitySlider->GetValue() - 0.1f) / 2.9f;
		MouseSensitivityBar->SetPercent(NormalizedValue);
	}

	// Camera zoom sensitivity (already 0.0-1.0)
	if (CameraZoomSensitivityBar && CameraZoomSensitivitySlider)
	{
		CameraZoomSensitivityBar->SetPercent(CameraZoomSensitivitySlider->GetValue());
	}

	// Gamepad sensitivity (normalize 0.1-3.0 to 0.0-1.0)
	if (GamepadSensitivityBar && GamepadSensitivitySlider)
	{
		float NormalizedValue = (GamepadSensitivitySlider->GetValue() - 0.1f) / 2.9f;
		GamepadSensitivityBar->SetPercent(NormalizedValue);
	}

	// Gamepad deadzone (normalize 0.0-0.5 to 0.0-1.0)
	if (GamepadDeadzoneBar && GamepadDeadzoneSlider)
	{
		float NormalizedValue = GamepadDeadzoneSlider->GetValue() / 0.5f;
		GamepadDeadzoneBar->SetPercent(NormalizedValue);
	}

	// Gamepad vibration intensity (already 0.0-1.0)
	if (GamepadVibrationIntensityBar && GamepadVibrationIntensitySlider)
	{
		GamepadVibrationIntensityBar->SetPercent(GamepadVibrationIntensitySlider->GetValue());
	}
}

void UControlWidget::BindSliderEvents()
{
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlWidget::OnMouseSensitivityChanged);
	}

	if (CameraZoomSensitivitySlider)
	{
		CameraZoomSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlWidget::OnCameraZoomSensitivityChanged);
	}

	if (GamepadSensitivitySlider)
	{
		GamepadSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlWidget::OnGamepadSensitivityChanged);
	}

	if (GamepadDeadzoneSlider)
	{
		GamepadDeadzoneSlider->OnValueChanged.AddDynamic(this, &UControlWidget::OnGamepadDeadzoneChanged);
	}

	if (GamepadVibrationIntensitySlider)
	{
		GamepadVibrationIntensitySlider->OnValueChanged.AddDynamic(this, &UControlWidget::OnGamepadVibrationIntensityChanged);
	}
}

void UControlWidget::UnbindSliderEvents()
{
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UControlWidget::OnMouseSensitivityChanged);
	}

	if (CameraZoomSensitivitySlider)
	{
		CameraZoomSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UControlWidget::OnCameraZoomSensitivityChanged);
	}

	if (GamepadSensitivitySlider)
	{
		GamepadSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UControlWidget::OnGamepadSensitivityChanged);
	}

	if (GamepadDeadzoneSlider)
	{
		GamepadDeadzoneSlider->OnValueChanged.RemoveDynamic(this, &UControlWidget::OnGamepadDeadzoneChanged);
	}

	if (GamepadVibrationIntensitySlider)
	{
		GamepadVibrationIntensitySlider->OnValueChanged.RemoveDynamic(this, &UControlWidget::OnGamepadVibrationIntensityChanged);
	}
}