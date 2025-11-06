// AudioWidget.cpp
#include "AudioWidget.h"
#include "EscapeIT/UI/Settings/Row/SelectionSettingRow.h"
#include "EscapeIT/UI/Settings/Row/NumericSettingRow.h"
#include "EscapeIT/Settings/Core/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UAudioWidget::UAudioWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsLoadingSettings(false)
    , SettingsSubsystem(nullptr)
    , TestSound(nullptr)
{
}

void UAudioWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get Settings Subsystem (optional)
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
    }

    if (!SettingsSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioWidget: SettingsSubsystem not found; widget will still work locally"));
    }

    // Initialize selection rows and bind their delegates
    InitializeSelectionRows();

    // Initialize numeric rows (bind delegates)
    if (MasterVolumeRow)
    {
        MasterVolumeRow->InitializeRow(0.0f, 1.0f, 0.01f, CurrentSettings.MasterVolume, FText::FromString(TEXT("Master Volume")));
        MasterVolumeRow->OnNumericValueChanged.AddDynamic(this, &UAudioWidget::HandleMasterVolumeRowChanged);
    }

    if (SFXVolumeRow)
    {
        SFXVolumeRow->InitializeRow(0.0f, 1.0f, 0.01f, CurrentSettings.SFXVolume, FText::FromString(TEXT("SFX Volume")));
        SFXVolumeRow->OnNumericValueChanged.AddDynamic(this, &UAudioWidget::HandleSFXVolumeRowChanged);
    }

    if (MusicVolumeRow)
    {
        MusicVolumeRow->InitializeRow(0.0f, 1.0f, 0.01f, CurrentSettings.MusicVolume, FText::FromString(TEXT("Music Volume")));
        MusicVolumeRow->OnNumericValueChanged.AddDynamic(this, &UAudioWidget::HandleMusicVolumeRowChanged);
    }

    if (AmbientVolumeRow)
    {
        AmbientVolumeRow->InitializeRow(0.0f, 1.0f, 0.01f, CurrentSettings.AmbientVolume, FText::FromString(TEXT("Ambient Volume")));
        AmbientVolumeRow->OnNumericValueChanged.AddDynamic(this, &UAudioWidget::HandleAmbientVolumeRowChanged);
    }

    if (DialogueVolumeRow)
    {
        DialogueVolumeRow->InitializeRow(0.0f, 1.0f, 0.01f, CurrentSettings.DialogueVolume, FText::FromString(TEXT("Dialogue Volume")));
        DialogueVolumeRow->OnNumericValueChanged.AddDynamic(this, &UAudioWidget::HandleDialogueVolumeRowChanged);
    }

    if (UIVolumeRow)
    {
        UIVolumeRow->InitializeRow(0.0f, 1.0f, 0.01f, CurrentSettings.UIVolume, FText::FromString(TEXT("UI Volume")));
        UIVolumeRow->OnNumericValueChanged.AddDynamic(this, &UAudioWidget::HandleUIVolumeRowChanged);
    }

    // Load current settings either from subsystem (if available) or defaults
    if (SettingsSubsystem)
    {
        LoadSettings(SettingsSubsystem->GetAllSettings().AudioSettings);
    }
    else
    {
        FS_AudioSettings DefaultSettings;
        LoadSettings(DefaultSettings);
    }

    // Bind test audio button
    if (TestAudioButton)
    {
        TestAudioButton->OnClicked.AddDynamic(this, &UAudioWidget::OnTestAudioButtonClicked);
    }

    // Load test sound (adjust path to your actual test sound asset)
    TestSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/UI/TestSound"));
}

void UAudioWidget::NativeDestruct()
{
    // Unbind selection row delegates
    if (AudioLanguageRow)
        AudioLanguageRow->OnSelectionChanged.RemoveDynamic(this, &UAudioWidget::OnAudioLanguageChanged);
    if (AudioOutputRow)
        AudioOutputRow->OnSelectionChanged.RemoveDynamic(this, &UAudioWidget::OnAudioOutputChanged);
    if (ClosedCaptionsRow)
        ClosedCaptionsRow->OnSelectionChanged.RemoveDynamic(this, &UAudioWidget::OnClosedCaptionsChanged);
    if (SubtitlesRow)
        SubtitlesRow->OnSelectionChanged.RemoveDynamic(this, &UAudioWidget::OnSubtitlesChanged);

    // Unbind numeric row delegates
    if (MasterVolumeRow)
        MasterVolumeRow->OnNumericValueChanged.RemoveDynamic(this, &UAudioWidget::HandleMasterVolumeRowChanged);
    if (SFXVolumeRow)
        SFXVolumeRow->OnNumericValueChanged.RemoveDynamic(this, &UAudioWidget::HandleSFXVolumeRowChanged);
    if (MusicVolumeRow)
        MusicVolumeRow->OnNumericValueChanged.RemoveDynamic(this, &UAudioWidget::HandleMusicVolumeRowChanged);
    if (AmbientVolumeRow)
        AmbientVolumeRow->OnNumericValueChanged.RemoveDynamic(this, &UAudioWidget::HandleAmbientVolumeRowChanged);
    if (DialogueVolumeRow)
        DialogueVolumeRow->OnNumericValueChanged.RemoveDynamic(this, &UAudioWidget::HandleDialogueVolumeRowChanged);
    if (UIVolumeRow)
        UIVolumeRow->OnNumericValueChanged.RemoveDynamic(this, &UAudioWidget::HandleUIVolumeRowChanged);

    // Unbind button
    if (TestAudioButton)
        TestAudioButton->OnClicked.RemoveDynamic(this, &UAudioWidget::OnTestAudioButtonClicked);

    Super::NativeDestruct();
}

// Initialize selection rows with options and bind delegates
void UAudioWidget::InitializeSelectionRows()
{
    const TArray<FText> ToggleOptions = MakeToggleOptions();
    const TArray<FText> AudioLanguageOptions = MakeAudioLanguageOptions();
    const TArray<FText> AudioOutputOptions = MakeAudioOutputOptions();

    if (AudioLanguageRow)
    {
        AudioLanguageRow->InitializeRow(AudioLanguageOptions, static_cast<int32>(CurrentSettings.CurrentLanguage), FText::FromString(TEXT("Audio Language")));
        AudioLanguageRow->OnSelectionChanged.AddDynamic(this, &UAudioWidget::OnAudioLanguageChanged);
    }

    if (AudioOutputRow)
    {
        AudioOutputRow->InitializeRow(AudioOutputOptions, static_cast<int32>(CurrentSettings.AudioOutput), FText::FromString(TEXT("Audio Output")));
        AudioOutputRow->OnSelectionChanged.AddDynamic(this, &UAudioWidget::OnAudioOutputChanged);
    }

    if (ClosedCaptionsRow)
    {
        ClosedCaptionsRow->InitializeRow(ToggleOptions, CurrentSettings.bClosedCaptionsEnabled ? 1 : 0, FText::FromString(TEXT("Closed Captions")));
        ClosedCaptionsRow->OnSelectionChanged.AddDynamic(this, &UAudioWidget::OnClosedCaptionsChanged);
    }

    if (SubtitlesRow)
    {
        SubtitlesRow->InitializeRow(ToggleOptions, CurrentSettings.bSubtitlesEnabled ? 1 : 0, FText::FromString(TEXT("Subtitles")));
        SubtitlesRow->OnSelectionChanged.AddDynamic(this, &UAudioWidget::OnSubtitlesChanged);
    }
}

TArray<FText> UAudioWidget::MakeToggleOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Off")));
    Out.Add(FText::FromString(TEXT("On")));
    return Out;
}

TArray<FText> UAudioWidget::MakeAudioLanguageOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("English")));
    Out.Add(FText::FromString(TEXT("Vietnamese")));
    Out.Add(FText::FromString(TEXT("Japanese")));
    Out.Add(FText::FromString(TEXT("Korean")));
    Out.Add(FText::FromString(TEXT("Chinese")));
    Out.Add(FText::FromString(TEXT("French")));
    Out.Add(FText::FromString(TEXT("German")));
    Out.Add(FText::FromString(TEXT("Spanish")));
    return Out;
}

TArray<FText> UAudioWidget::MakeAudioOutputOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Stereo")));
    Out.Add(FText::FromString(TEXT("Surround 5.1")));
    Out.Add(FText::FromString(TEXT("Surround 7.1")));
    Out.Add(FText::FromString(TEXT("Headphones")));
    return Out;
}

void UAudioWidget::LoadSettings(const FS_AudioSettings& Settings)
{
    bIsLoadingSettings = true;
    CurrentSettings = Settings;

    // Update numeric rows (do not trigger delegates)
    if (MasterVolumeRow)
        MasterVolumeRow->SetValue(CurrentSettings.MasterVolume, false);

    if (SFXVolumeRow)
        SFXVolumeRow->SetValue(CurrentSettings.SFXVolume, false);

    if (MusicVolumeRow)
        MusicVolumeRow->SetValue(CurrentSettings.MusicVolume, false);

    if (AmbientVolumeRow)
        AmbientVolumeRow->SetValue(CurrentSettings.AmbientVolume, false);

    if (DialogueVolumeRow)
        DialogueVolumeRow->SetValue(CurrentSettings.DialogueVolume, false);

    if (UIVolumeRow)
        UIVolumeRow->SetValue(CurrentSettings.UIVolume, false);

    // Update selection rows (do not trigger delegates)
    if (AudioLanguageRow)
        AudioLanguageRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.CurrentLanguage), false);

    if (AudioOutputRow)
        AudioOutputRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.AudioOutput), false);

    if (ClosedCaptionsRow)
        ClosedCaptionsRow->SetCurrentSelection(CurrentSettings.bClosedCaptionsEnabled ? 1 : 0, false);

    if (SubtitlesRow)
        SubtitlesRow->SetCurrentSelection(CurrentSettings.bSubtitlesEnabled ? 1 : 0, false);

    bIsLoadingSettings = false;
}

FS_AudioSettings UAudioWidget::GetCurrentSettings() const
{
    return CurrentSettings;
}

TArray<FString> UAudioWidget::ValidateSettings() const
{
    TArray<FString> Errors;

    if (CurrentSettings.MasterVolume < 0.0f || CurrentSettings.MasterVolume > 1.0f)
    {
        Errors.Add(TEXT("Master volume out of range (0.0 - 1.0)"));
    }
    if (CurrentSettings.SFXVolume < 0.0f || CurrentSettings.SFXVolume > 1.0f)
    {
        Errors.Add(TEXT("SFX volume out of range (0.0 - 1.0)"));
    }
    if (CurrentSettings.MusicVolume < 0.0f || CurrentSettings.MusicVolume > 1.0f)
    {
        Errors.Add(TEXT("Music volume out of range (0.0 - 1.0)"));
    }
    if (CurrentSettings.AmbientVolume < 0.0f || CurrentSettings.AmbientVolume > 1.0f)
    {
        Errors.Add(TEXT("Ambient volume out of range (0.0 - 1.0)"));
    }
    if (CurrentSettings.DialogueVolume < 0.0f || CurrentSettings.DialogueVolume > 1.0f)
    {
        Errors.Add(TEXT("Dialogue volume out of range (0.0 - 1.0)"));
    }
    if (CurrentSettings.UIVolume < 0.0f || CurrentSettings.UIVolume > 1.0f)
    {
        Errors.Add(TEXT("UI volume out of range (0.0 - 1.0)"));
    }

    if (Errors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioWidget: Validation found %d errors"), Errors.Num());
    }

    return Errors;
}

// Selection callbacks

void UAudioWidget::OnAudioLanguageChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.CurrentLanguage = static_cast<EE_AudioLanguage>(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: AudioLanguage changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::OnAudioOutputChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.AudioOutput = static_cast<EE_AudioOutput>(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: AudioOutput changed to index %d (local)"), NewIndex);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::OnClosedCaptionsChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bClosedCaptionsEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: ClosedCaptions changed to %s (local)"), CurrentSettings.bClosedCaptionsEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::OnSubtitlesChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bSubtitlesEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: Subtitles changed to %s (local)"), CurrentSettings.bSubtitlesEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

// Numeric row handlers

void UAudioWidget::HandleMasterVolumeRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.MasterVolume = NewValue;
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: MasterVolume updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::HandleSFXVolumeRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.SFXVolume = NewValue;
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: SFXVolume updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::HandleMusicVolumeRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.MusicVolume = NewValue;
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: MusicVolume updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::HandleAmbientVolumeRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.AmbientVolume = NewValue;
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: AmbientVolume updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::HandleDialogueVolumeRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.DialogueVolume = NewValue;
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: DialogueVolume updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::HandleUIVolumeRowChanged(float NewValue)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.UIVolume = NewValue;
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: UIVolume updated -> %.3f"), NewValue);
    if (SettingsSubsystem) SettingsSubsystem->ApplyAudioSettings(CurrentSettings);
}

void UAudioWidget::OnTestAudioButtonClicked()
{
    if (TestSound)
    {
        UGameplayStatics::PlaySound2D(this, TestSound);
        UE_LOG(LogTemp, Log, TEXT("AudioWidget: Playing test sound"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioWidget: Test sound not loaded"));
    }
}