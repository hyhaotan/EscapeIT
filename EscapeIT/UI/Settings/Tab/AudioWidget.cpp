// AudioWidget.cpp
#include "AudioWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Settings/Core/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UAudioWidget::UAudioWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUpdatingSliders(false)
	, bIsLoadingSettings(false)
	, SettingsSubsystem(nullptr)
	, TestSound(nullptr)
{
}

void UAudioWidget::NativeConstruct()
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
		UE_LOG(LogTemp, Warning, TEXT("AudioWidget: SettingsSubsystem not found; widget will still work locally"));
	}

	// Initialize UI
	InitializeSelections();
	InitializeSliders();
	BindSliderEvents();

	// Load current settings from subsystem if available, otherwise defaults
	if (SettingsSubsystem)
	{
		LoadSettings(SettingsSubsystem->GetAllSettings().AudioSettings);
	}
	else
	{
		FS_AudioSettings DefaultSettings;
		LoadSettings(DefaultSettings);
	}

	// Bind buttons
	if (TestAudioButton)
	{
		TestAudioButton->OnClicked.AddDynamic(this, &UAudioWidget::OnTestAudioButtonClicked);
	}

	// Load test sound (adjust path to your actual test sound asset)
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

void UAudioWidget::LoadSettings(const FS_AudioSettings& Settings)
{
	// Prevent callbacks while we populate UI
	bIsLoadingSettings = true;
	CurrentSettings = Settings;

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

	// Selections
	if (AudioLanguageSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.CurrentLanguage);
		AudioLanguageSelection->SetCurrentSelection(Index);
	}

	if (AudioOutputSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.AudioOutput);
		AudioOutputSelection->SetCurrentSelection(Index);
	}

	if (ClosedCaptionsSelection)
	{
		ClosedCaptionsSelection->SetCurrentSelection(CurrentSettings.bClosedCaptionsEnabled ? 1 : 0);
	}

	if (SubtitlesSelection)
	{
		SubtitlesSelection->SetCurrentSelection(CurrentSettings.bSubtitlesEnabled ? 1 : 0);
	}

	bIsLoadingSettings = false;
	UpdateVolumeTexts();

	UE_LOG(LogTemp, Log, TEXT("AudioWidget: Settings loaded into UI (local only)"));
}

FS_AudioSettings UAudioWidget::GetCurrentSettings() const
{
	return CurrentSettings;
}

TArray<FString> UAudioWidget::ValidateSettings() const
{
	TArray<FString> Errors;
	// Validate ranges
	if (CurrentSettings.MasterVolume < 0.0f || CurrentSettings.MasterVolume > 1.0f)
	{
		Errors.Add(TEXT("Master volume out of range (0.0 - 1.0)"));
	}
	if (CurrentSettings.SFXVolume < 0.0f || CurrentSettings.SFXVolume > 1.0f)
	{
		Errors.Add(TEXT("SFX volume out of range (0.0 - 1.0)"));
	}
	// Add more checks as needed

	if (Errors.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AudioWidget: Validation found %d errors"), Errors.Num());
	}
	return Errors;
}

// ===== CALLBACKS (update CurrentSettings only) =====

void UAudioWidget::OnMasterVolumeChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.MasterVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	UpdateVolumeTexts();
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: MasterVolume changed to %.3f (local, not saved)"), CurrentSettings.MasterVolume);
}

void UAudioWidget::OnSFXVolumeChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.SFXVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	UpdateVolumeTexts();
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: SFXVolume changed to %.3f (local, not saved)"), CurrentSettings.SFXVolume);
}

void UAudioWidget::OnMusicVolumeChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.MusicVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	UpdateVolumeTexts();
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: MusicVolume changed to %.3f (local, not saved)"), CurrentSettings.MusicVolume);
}

void UAudioWidget::OnAmbientVolumeChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.AmbientVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	UpdateVolumeTexts();
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: AmbientVolume changed to %.3f (local, not saved)"), CurrentSettings.AmbientVolume);
}

void UAudioWidget::OnDialogueVolumeChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.DialogueVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	UpdateVolumeTexts();
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: DialogueVolume changed to %.3f (local, not saved)"), CurrentSettings.DialogueVolume);
}

void UAudioWidget::OnUIVolumeChanged(float Value)
{
	if (bIsLoadingSettings || bUpdatingSliders)
		return;

	CurrentSettings.UIVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	UpdateVolumeTexts();
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: UIVolume changed to %.3f (local, not saved)"), CurrentSettings.UIVolume);
}

void UAudioWidget::OnAudioLanguageChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.CurrentLanguage = static_cast<EE_AudioLanguage>(NewIndex);
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: AudioLanguage changed to index %d (local)"), NewIndex);
}

void UAudioWidget::OnAudioOutputChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.AudioOutput = static_cast<EE_AudioOutput>(NewIndex);
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: AudioOutput changed to index %d (local)"), NewIndex);
}

void UAudioWidget::OnClosedCaptionsChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bClosedCaptionsEnabled = (NewIndex == 1);
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: ClosedCaptions changed to %s (local)"), CurrentSettings.bClosedCaptionsEnabled ? TEXT("true") : TEXT("false"));
}

void UAudioWidget::OnSubtitlesChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bSubtitlesEnabled = (NewIndex == 1);
	UE_LOG(LogTemp, Log, TEXT("AudioWidget: Subtitles changed to %s (local)"), CurrentSettings.bSubtitlesEnabled ? TEXT("true") : TEXT("false"));
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

// ===== HELPERS =====

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
	// Show numeric values (0.0 - 1.0). If you prefer percentage, multiply by 100.
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
