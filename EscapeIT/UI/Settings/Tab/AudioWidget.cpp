// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Subsystem/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UAudioWidget::UAudioWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUpdatingSliders(false)
{
}

void UAudioWidget::NativeConstruct()
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
		UE_LOG(LogTemp, Error, TEXT("AudioWidget: Failed to get SettingsSubsystem"));
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
	if (TestAudioButton)
	{
		TestAudioButton->OnClicked.AddDynamic(this, &UAudioWidget::OnTestAudioButtonClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UAudioWidget::OnResetButtonClicked);
	}

	// Load test sound (you should set this path to your actual test sound)
	TestSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/UI/TestSound"));
}

void UAudioWidget::NativeDestruct()
{
	// Unbind all events
	UnbindSliderEvents();

	if (TestAudioButton)
	{
		TestAudioButton->OnClicked.RemoveDynamic(this, &UAudioWidget::OnTestAudioButtonClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UAudioWidget::OnResetButtonClicked);
	}

	Super::NativeDestruct();
}

void UAudioWidget::InitializeSelections()
{
	// Audio Language
	if (AudioLanguageSelection)
	{
		AudioLanguageSelection->Clear();
		AudioLanguageSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("English")) });
		AudioLanguageSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Vietnamese")) });
		AudioLanguageSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Japanese")) });
		AudioLanguageSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Korean")) });
		AudioLanguageSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Chinese")) });
		AudioLanguageSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("French")) });
		AudioLanguageSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("German")) });
		AudioLanguageSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Spanish")) });
		AudioLanguageSelection->OnSelectionChanged.BindDynamic(this, &UAudioWidget::OnAudioLanguageChanged);
	}

	// Audio Output
	if (AudioOutputSelection)
	{
		AudioOutputSelection->Clear();
		AudioOutputSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Stereo")) });
		AudioOutputSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Surround 5.1")) });
		AudioOutputSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Surround 7.1")) });
		AudioOutputSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Headphones")) });
		AudioOutputSelection->OnSelectionChanged.BindDynamic(this, &UAudioWidget::OnAudioOutputChanged);
	}

	// Closed Captions
	if (ClosedCaptionsSelection)
	{
		ClosedCaptionsSelection->Clear();
		AddToggleOptions(ClosedCaptionsSelection);
		ClosedCaptionsSelection->OnSelectionChanged.BindDynamic(this, &UAudioWidget::OnClosedCaptionsChanged);
	}

	// Subtitles
	if (SubtitlesSelection)
	{
		SubtitlesSelection->Clear();
		AddToggleOptions(SubtitlesSelection);
		SubtitlesSelection->OnSelectionChanged.BindDynamic(this, &UAudioWidget::OnSubtitlesChanged);
	}
}

void UAudioWidget::InitializeSliders()
{
	// Set slider ranges (0.0 to 1.0)
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetMinValue(0.0f);
		MasterVolumeSlider->SetMaxValue(1.0f);
		MasterVolumeSlider->SetStepSize(0.01f);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->SetMinValue(0.0f);
		SFXVolumeSlider->SetMaxValue(1.0f);
		SFXVolumeSlider->SetStepSize(0.01f);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->SetMinValue(0.0f);
		MusicVolumeSlider->SetMaxValue(1.0f);
		MusicVolumeSlider->SetStepSize(0.01f);
	}

	if (AmbientVolumeSlider)
	{
		AmbientVolumeSlider->SetMinValue(0.0f);
		AmbientVolumeSlider->SetMaxValue(1.0f);
		AmbientVolumeSlider->SetStepSize(0.01f);
	}

	if (DialogueVolumeSlider)
	{
		DialogueVolumeSlider->SetMinValue(0.0f);
		DialogueVolumeSlider->SetMaxValue(1.0f);
		DialogueVolumeSlider->SetStepSize(0.01f);
	}

	if (UIVolumeSlider)
	{
		UIVolumeSlider->SetMinValue(0.0f);
		UIVolumeSlider->SetMaxValue(1.0f);
		UIVolumeSlider->SetStepSize(0.01f);
	}
}

void UAudioWidget::LoadCurrentSettings()
{
	if (!SettingsSubsystem)
		return;

	bUpdatingSliders = true; // Prevent feedback loop

	FS_AudioSettings CurrentSettings = SettingsSubsystem->GetAllSettings().AudioSettings;

	// Set slider values
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(CurrentSettings.MasterVolume);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->SetValue(CurrentSettings.SFXVolume);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->SetValue(CurrentSettings.MusicVolume);
	}

	if (AmbientVolumeSlider)
	{
		AmbientVolumeSlider->SetValue(CurrentSettings.AmbientVolume);
	}

	if (DialogueVolumeSlider)
	{
		DialogueVolumeSlider->SetValue(CurrentSettings.DialogueVolume);
	}

	if (UIVolumeSlider)
	{
		UIVolumeSlider->SetValue(CurrentSettings.UIVolume);
	}

	// Audio Language
	if (AudioLanguageSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.CurrentLanguage);
		AudioLanguageSelection->SetCurrentSelection(Index);
	}

	// Audio Output
	if (AudioOutputSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.AudioOutput);
		AudioOutputSelection->SetCurrentSelection(Index);
	}

	// Closed Captions
	if (ClosedCaptionsSelection)
	{
		ClosedCaptionsSelection->SetCurrentSelection(CurrentSettings.bClosedCaptionsEnabled ? 1 : 0);
	}

	// Subtitles
	if (SubtitlesSelection)
	{
		SubtitlesSelection->SetCurrentSelection(CurrentSettings.bSubtitlesEnabled ? 1 : 0);
	}

	bUpdatingSliders = false;
	UpdateVolumeTexts();
}

// ===== CALLBACKS =====

void UAudioWidget::OnMasterVolumeChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetMasterVolume(Value);
	}
	UpdateVolumeTexts();
}

void UAudioWidget::OnSFXVolumeChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetSFXVolume(Value);
	}
	UpdateVolumeTexts();
}

void UAudioWidget::OnMusicVolumeChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetMusicVolume(Value);
	}
	UpdateVolumeTexts();
}

void UAudioWidget::OnAmbientVolumeChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetAmbientVolume(Value);
	}
	UpdateVolumeTexts();
}

void UAudioWidget::OnDialogueVolumeChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetDialogueVolume(Value);
	}
	UpdateVolumeTexts();
}

void UAudioWidget::OnUIVolumeChanged(float Value)
{
	if (SettingsSubsystem && !bUpdatingSliders)
	{
		SettingsSubsystem->SetUIVolume(Value);
	}
	UpdateVolumeTexts();
}

void UAudioWidget::OnAudioLanguageChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_AudioLanguage Language = static_cast<EE_AudioLanguage>(NewIndex);
		SettingsSubsystem->SetAudioLanguage(Language);
	}
}

void UAudioWidget::OnAudioOutputChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_AudioOutput Output = static_cast<EE_AudioOutput>(NewIndex);
		SettingsSubsystem->SetAudioOutput(Output);
	}
}

void UAudioWidget::OnClosedCaptionsChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetClosedCaptionsEnabled(NewIndex == 1);
	}
}

void UAudioWidget::OnSubtitlesChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetSubtitlesEnabled(NewIndex == 1);
	}
}

void UAudioWidget::OnTestAudioButtonClicked()
{
	if (TestSound)
	{
		UGameplayStatics::PlaySound2D(this, TestSound);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AudioWidget: Test sound not loaded"));
	}
}

void UAudioWidget::OnResetButtonClicked()
{
	if (SettingsSubsystem)
	{
		// Reset only audio settings to default
		FS_AudioSettings DefaultSettings;
		SettingsSubsystem->ApplyAudioSettings(DefaultSettings);

		// Reload UI
		LoadCurrentSettings();
	}
}

// ===== HELPER FUNCTIONS =====

void UAudioWidget::AddToggleOptions(USelectionWidget* Selection)
{
	if (Selection)
	{
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("Off")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("On")) });
	}
}

void UAudioWidget::UpdateVolumeTexts()
{
	// Update text displays to show volume percentage
	if (MasterVolumeText && MasterVolumeSlider)
	{
		float Value = MasterVolumeSlider->GetValue();
		MasterVolumeText->SetText(FText::AsNumber(Value));
	}

	if (SFXVolumeText && SFXVolumeSlider)
	{
		float Value = SFXVolumeSlider->GetValue();
		SFXVolumeText->SetText(FText::AsNumber(Value));
	}

	if (MusicVolumeText && MusicVolumeSlider)
	{
		float Value = MusicVolumeSlider->GetValue();
		MusicVolumeText->SetText(FText::AsNumber(Value));
	}

	if (AmbientVolumeText && AmbientVolumeSlider)
	{
		float Value = AmbientVolumeSlider->GetValue();
		AmbientVolumeText->SetText(FText::AsNumber(Value));
	}

	if (DialogueVolumeText && DialogueVolumeSlider)
	{
		float Value = DialogueVolumeSlider->GetValue();
		DialogueVolumeText->SetText(FText::AsNumber(Value));
	}

	if (UIVolumeText && UIVolumeSlider)
	{
		float Value = UIVolumeSlider->GetValue();
		UIVolumeText->SetText(FText::AsNumber(Value));
	}
}

void UAudioWidget::BindSliderEvents()
{
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnMasterVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnSFXVolumeChanged);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnMusicVolumeChanged);
	}

	if (AmbientVolumeSlider)
	{
		AmbientVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnAmbientVolumeChanged);
	}

	if (DialogueVolumeSlider)
	{
		DialogueVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnDialogueVolumeChanged);
	}

	if (UIVolumeSlider)
	{
		UIVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnUIVolumeChanged);
	}
}

void UAudioWidget::UnbindSliderEvents()
{
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.RemoveDynamic(this, &UAudioWidget::OnMasterVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.RemoveDynamic(this, &UAudioWidget::OnSFXVolumeChanged);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.RemoveDynamic(this, &UAudioWidget::OnMusicVolumeChanged);
	}

	if (AmbientVolumeSlider)
	{
		AmbientVolumeSlider->OnValueChanged.RemoveDynamic(this, &UAudioWidget::OnAmbientVolumeChanged);
	}

	if (DialogueVolumeSlider)
	{
		DialogueVolumeSlider->OnValueChanged.RemoveDynamic(this, &UAudioWidget::OnDialogueVolumeChanged);
	}

	if (UIVolumeSlider)
	{
		UIVolumeSlider->OnValueChanged.RemoveDynamic(this, &UAudioWidget::OnUIVolumeChanged);
	}
}