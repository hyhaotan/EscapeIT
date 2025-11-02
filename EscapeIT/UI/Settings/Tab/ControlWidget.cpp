// ControlWidget.cpp
#include "ControlWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Settings/Core/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

UControlWidget::UControlWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUpdatingSliders(false)
	, bIsLoadingSettings(false)
	, SettingsSubsystem(nullptr)
{
}

void UControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get Settings Subsystem (optional)
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
	}

	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("ControlWidget: SettingsSubsystem not found; widget will still work locally"));
	}

	// Initialize UI elements
	InitializeSelections();
	InitializeSliders();
	BindSliderEvents();

	// Load current settings either from subsystem (if available) or from defaults
	if (SettingsSubsystem)
	{
		LoadSettings(SettingsSubsystem->GetAllSettings().ControlSettings);
	}
	else
	{
		FS_ControlSettings DefaultSettings;
		LoadSettings(DefaultSettings);
	}

	// Bind buttons
	if (RebindKeysButton)
	{
		RebindKeysButton->OnClicked.AddDynamic(this, &UControlWidget::OnRebindKeysButtonClicked);
	}
}

void UControlWidget::NativeDestruct()
{
	UnbindSliderEvents();

	if (RebindKeysButton)
	{
		RebindKeysButton->OnClicked.RemoveDynamic(this, &UControlWidget::OnRebindKeysButtonClicked);
	}

	Super::NativeDestruct();
}

void UControlWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bUpdatingSliders && !bIsLoadingSettings)
	{
		UpdateVolumeText();
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

void UControlWidget::LoadSettings(const FS_ControlSettings& Settings)
{
	// Use a loading flag to prevent callbacks from firing while setting UI values
	bIsLoadingSettings = true;
	CurrentSettings = Settings;

	// Mouse Settings
	if (MouseSensitivitySlider)
		MouseSensitivitySlider->SetValue(CurrentSettings.MouseSensitivity);

	if (InvertMouseYSelection)
		InvertMouseYSelection->SetCurrentSelection(CurrentSettings.bInvertMouseY ? 1 : 0);

	if (CameraZoomSensitivitySlider)
		CameraZoomSensitivitySlider->SetValue(CurrentSettings.CameraZoomSensitivity);

	// Gamepad Settings
	if (GamepadSensitivitySlider)
		GamepadSensitivitySlider->SetValue(CurrentSettings.GamepadSensitivity);

	if (GamepadDeadzoneSlider)
		GamepadDeadzoneSlider->SetValue(CurrentSettings.GamepadDeadzone);

	if (InvertGamepadYSelection)
		InvertGamepadYSelection->SetCurrentSelection(CurrentSettings.bInvertGamepadY ? 1 : 0);

	if (GamepadVibrationSelection)
		GamepadVibrationSelection->SetCurrentSelection(CurrentSettings.bGamepadVibrationEnabled ? 1 : 0);

	if (GamepadVibrationIntensitySlider)
		GamepadVibrationIntensitySlider->SetValue(CurrentSettings.GamepadVibrationIntensity);

	// Gameplay Controls
	if (AutoSprintSelection)
		AutoSprintSelection->SetCurrentSelection(CurrentSettings.bAutoSprintEnabled ? 1 : 0);

	if (CrouchToggleSelection)
		CrouchToggleSelection->SetCurrentSelection(CurrentSettings.bCrouchToggle ? 1 : 0);

	if (FlashlightToggleSelection)
		FlashlightToggleSelection->SetCurrentSelection(CurrentSettings.bFlashlightToggle ? 1 : 0);

	bIsLoadingSettings = false;

	// Update texts after loading
	UpdateVolumeText();

	UE_LOG(LogTemp, Log, TEXT("ControlWidget: Settings loaded into UI (local only)"));
}

FS_ControlSettings UControlWidget::GetCurrentSettings() const
{
	return CurrentSettings;
}

TArray<FString> UControlWidget::ValidateSettings() const
{
	TArray<FString> Errors;
	// Example: Validate ranges (add rules as needed)
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

// ===== MOUSE CALLBACKS =====

void UControlWidget::OnMouseSensitivityChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.MouseSensitivity = Value;
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: Mouse sensitivity changed to %.3f (local, not saved)"), Value);
}

void UControlWidget::OnCameraZoomSensitivityChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.CameraZoomSensitivity = Value;
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: Camera zoom sensitivity changed to %.3f (local, not saved)"), Value);
}

// ===== GAMEPAD CALLBACKS =====

void UControlWidget::OnGamepadSensitivityChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.GamepadSensitivity = Value;
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: Gamepad sensitivity changed to %.3f (local, not saved)"), Value);
}

void UControlWidget::OnGamepadDeadzoneChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.GamepadDeadzone = Value;
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: Gamepad deadzone changed to %.3f (local, not saved)"), Value);
}

void UControlWidget::OnGamepadVibrationIntensityChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.GamepadVibrationIntensity = Value;
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: Gamepad vibration intensity changed to %.3f (local, not saved)"), Value);
}

// ===== SELECTION CALLBACKS =====

void UControlWidget::OnInvertMouseYChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bInvertMouseY = (NewIndex == 1);
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: InvertMouseY changed to %s (local)"), CurrentSettings.bInvertMouseY ? TEXT("true") : TEXT("false"));
}

void UControlWidget::OnInvertGamepadYChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bInvertGamepadY = (NewIndex == 1);
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: InvertGamepadY changed to %s (local)"), CurrentSettings.bInvertGamepadY ? TEXT("true") : TEXT("false"));
}

void UControlWidget::OnGamepadVibrationChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bGamepadVibrationEnabled = (NewIndex == 1);
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: GamepadVibration changed to %s (local)"), CurrentSettings.bGamepadVibrationEnabled ? TEXT("true") : TEXT("false"));
}

void UControlWidget::OnAutoSprintChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bAutoSprintEnabled = (NewIndex == 1);
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: AutoSprint changed to %s (local)"), CurrentSettings.bAutoSprintEnabled ? TEXT("true") : TEXT("false"));
}

void UControlWidget::OnCrouchToggleChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bCrouchToggle = (NewIndex == 1);
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: CrouchToggle changed to %s (local)"), CurrentSettings.bCrouchToggle ? TEXT("true") : TEXT("false"));
}

void UControlWidget::OnFlashlightToggleChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bFlashlightToggle = (NewIndex == 1);
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: FlashlightToggle changed to %s (local)"), CurrentSettings.bFlashlightToggle ? TEXT("true") : TEXT("false"));
}

// ===== BUTTON CALLBACKS =====

void UControlWidget::OnRebindKeysButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("ControlWidget: Rebind Keys clicked - open key binding UI (not implemented here)"));
	// No subsystem call here — this should open a rebind widget / broadcast an event
}

// ===== HELPERS =====

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

void UControlWidget::UpdateVolumeText()
{
	if (MouseSensitivityText && MouseSensitivitySlider)
	{
		float Value = MouseSensitivitySlider->GetValue();
		MouseSensitivityText->SetText(FText::AsNumber(Value));
	}

	if (CameraZoomSensitivityText && CameraZoomSensitivitySlider)
	{
		float Value = CameraZoomSensitivitySlider->GetValue();
		CameraZoomSensitivityText->SetText(FText::AsNumber(Value));
	}

	if (GamepadSensitivityText && GamepadSensitivitySlider)
	{
		float Value = GamepadSensitivitySlider->GetValue();
		GamepadSensitivityText->SetText(FText::AsNumber(Value));
	}

	if (GamepadDeadzoneText && GamepadDeadzoneSlider)
	{
		float Value = GamepadDeadzoneSlider->GetValue();
		GamepadDeadzoneText->SetText(FText::AsNumber(Value));
	}

	if (GamepadVibrationIntensityText && GamepadVibrationIntensitySlider)
	{
		float Value = GamepadVibrationIntensitySlider->GetValue();
		GamepadVibrationIntensityText->SetText(FText::AsNumber(Value));
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
