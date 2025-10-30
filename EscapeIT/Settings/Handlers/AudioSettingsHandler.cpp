#include "AudioSettingsHandler.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

// Static members
USoundMix* FAudioSettingsHandler::MasterSoundMix = nullptr;
USoundClass* FAudioSettingsHandler::MasterSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::MusicSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::SFXSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::AmbientSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::DialogueSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::UISoundClass = nullptr;

void FAudioSettingsHandler::Initialize(UWorld* World)
{
    LoadSoundAssets();
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Initialized"));
}

void FAudioSettingsHandler::ApplyToEngine(const FS_AudioSettings& Settings, UWorld* World)
{
    if (!MasterSoundMix)
    {
        Initialize(World);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Applying audio settings"));

    SetMasterVolume(Settings.MasterVolume);
    SetMusicVolume(Settings.MusicVolume);
    SetSFXVolume(Settings.SFXVolume);
    SetAmbientVolume(Settings.AmbientVolume);
    SetDialogueVolume(Settings.DialogueVolume);
    SetUIVolume(Settings.UIVolume);

    // Apply language and output settings
    // TODO: Implement language switching and audio output configuration

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: All audio settings applied"));
}

void FAudioSettingsHandler::SetMasterVolume(float Volume)
{
    ApplyVolumeToSoundClass(MasterSoundClass, Volume);
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Master volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetMusicVolume(float Volume)
{
    ApplyVolumeToSoundClass(MusicSoundClass, Volume);
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Music volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetSFXVolume(float Volume)
{
    ApplyVolumeToSoundClass(SFXSoundClass, Volume);
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: SFX volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetAmbientVolume(float Volume)
{
    ApplyVolumeToSoundClass(AmbientSoundClass, Volume);
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Ambient volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetDialogueVolume(float Volume)
{
    ApplyVolumeToSoundClass(DialogueSoundClass, Volume);
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Dialogue volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetUIVolume(float Volume)
{
    ApplyVolumeToSoundClass(UISoundClass, Volume);
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: UI volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::LoadSoundAssets()
{
    // Load Sound Mix and Sound Classes from Content
    // NOTE: You need to create
}

void FAudioSettingsHandler::ApplyVolumeToSoundClass(USoundClass* SoundClass, float Volume)
{
}
