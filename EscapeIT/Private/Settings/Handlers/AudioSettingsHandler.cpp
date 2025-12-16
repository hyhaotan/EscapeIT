#include "Settings/Handlers/AudioSettingsHandler.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include <Settings/Core/SettingsSubsystem.h>

// Static member initialization
USoundMix* FAudioSettingsHandler::MasterSoundMix = nullptr;
USoundClass* FAudioSettingsHandler::MasterSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::MusicSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::SFXSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::AmbientSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::DialogueSoundClass = nullptr;
USoundClass* FAudioSettingsHandler::UISoundClass = nullptr;
FS_AudioSettings FAudioSettingsHandler::CachedAudioSettings = FS_AudioSettings();
FS_AllSettings FAudioSettingsHandler::AllSettings = FS_AllSettings();

float FAudioSettingsHandler::CachedMasterVolume = 1.0f;
float FAudioSettingsHandler::CachedMusicVolume = 1.0f;
float FAudioSettingsHandler::CachedSFXVolume = 1.0f;
float FAudioSettingsHandler::CachedAmbientVolume = 1.0f;
float FAudioSettingsHandler::CachedDialogueVolume = 1.0f;
float FAudioSettingsHandler::CachedUIVolume = 1.0f;

bool FAudioSettingsHandler::bIsInitialized = false;
bool FAudioSettingsHandler::bWasMutedBySubsystem = false;
bool FAudioSettingsHandler::bDirtyFlag = false;

// Asset paths - CONFIGURE THESE TO MATCH YOUR PROJECT
const FString FAudioSettingsHandler::MasterSoundClassPath = TEXT("/Game/Audio/SoundClasses/Master.Master");
const FString FAudioSettingsHandler::MusicSoundClassPath = TEXT("/Game/Audio/SoundClasses/Music.Music");
const FString FAudioSettingsHandler::SFXSoundClassPath = TEXT("/Game/Audio/SoundClasses/SFX.SFX");
const FString FAudioSettingsHandler::AmbientSoundClassPath = TEXT("/Game/Free_Horror_Music_Pack/Wav/Death_Threat_In_Daylight_Loop.Death_Threat_In_Daylight_Loop");
const FString FAudioSettingsHandler::DialogueSoundClassPath = TEXT("/Game/Audio/SoundClasses/Dialogue.Dialogue");
const FString FAudioSettingsHandler::UISoundClassPath = TEXT("/Game/Audio/SoundClasses/UI.UI");
const FString FAudioSettingsHandler::MasterSoundMixPath = TEXT("/Game/Audio/SoundMixes/MasterMix.MasterMix");

void FAudioSettingsHandler::Initialize(UWorld* World)
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioHandler: Already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Initializing..."));

    LoadSoundAssets();

    // Verify audio device
    FAudioDevice* AudioDevice = GetAudioDevice(World);
    if (!AudioDevice)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioHandler: Failed to get audio device"));
        return;
    }

    // Push the sound mix to start using it
    if (MasterSoundMix && World)
    {
        PushSoundMixModifier(World);
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Initialized successfully"));
}

void FAudioSettingsHandler::Shutdown()
{
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Shutting down"));

    // Clear references
    MasterSoundMix = nullptr;
    MasterSoundClass = nullptr;
    MusicSoundClass = nullptr;
    SFXSoundClass = nullptr;
    AmbientSoundClass = nullptr;
    DialogueSoundClass = nullptr;
    UISoundClass = nullptr;

    bIsInitialized = false;
}

void FAudioSettingsHandler::ApplyToEngine(const FS_AudioSettings& Settings, UWorld* World)
{
    if (!bIsInitialized)
    {
        Initialize(World);
    }

    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioHandler: Cannot apply settings - initialization failed"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Applying audio settings"));

    // Apply volumes
    SetMasterVolume(Settings.MasterVolume);
    SetMusicVolume(Settings.MusicVolume);
    SetSFXVolume(Settings.SFXVolume);
    SetAmbientVolume(Settings.AmbientVolume);
    SetDialogueVolume(Settings.DialogueVolume);
    SetUIVolume(Settings.UIVolume);

    // Apply advanced settings
    if (Settings.CurrentLanguage != EE_AudioLanguage::English)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioHandler: Language setting is not English"));
    }

    // Apply spatialization and reverb settings if needed
    // SetSpatializationMethod(Settings.SpatializationMethod);
    // SetReverbQuality(Settings.ReverbQuality);

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: All audio settings applied"));
}

void FAudioSettingsHandler::SetMasterVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CachedMasterVolume = Volume;

    ApplyVolumeToSoundClass(MasterSoundClass, Volume);

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Master volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetMusicVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CachedMusicVolume = Volume;

    ApplyVolumeToSoundClass(MusicSoundClass, Volume);

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Music volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetSFXVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CachedSFXVolume = Volume;

    ApplyVolumeToSoundClass(SFXSoundClass, Volume);

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: SFX volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetAmbientVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CachedAmbientVolume = Volume;

    ApplyVolumeToSoundClass(AmbientSoundClass, Volume);

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Ambient volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetDialogueVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CachedDialogueVolume = Volume;

    ApplyVolumeToSoundClass(DialogueSoundClass, Volume);

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Dialogue volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::SetUIVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CachedUIVolume = Volume;

    ApplyVolumeToSoundClass(UISoundClass, Volume);

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: UI volume set to %.2f"), Volume);
}

void FAudioSettingsHandler::LoadSoundAssets()
{
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Loading sound assets"));

    // Load Sound Mix
    MasterSoundMix = Cast<USoundMix>(StaticLoadObject(
        USoundMix::StaticClass(),
        nullptr,
        *MasterSoundMixPath
    ));

    if (!MasterSoundMix)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioHandler: Failed to load Master SoundMix from '%s'"),
            *MasterSoundMixPath);
    }

    // Load Sound Classes
    MasterSoundClass = Cast<USoundClass>(StaticLoadObject(
        USoundClass::StaticClass(),
        nullptr,
        *MasterSoundClassPath
    ));

    MusicSoundClass = Cast<USoundClass>(StaticLoadObject(
        USoundClass::StaticClass(),
        nullptr,
        *MusicSoundClassPath
    ));

    SFXSoundClass = Cast<USoundClass>(StaticLoadObject(
        USoundClass::StaticClass(),
        nullptr,
        *SFXSoundClassPath
    ));

    AmbientSoundClass = Cast<USoundClass>(StaticLoadObject(
        USoundClass::StaticClass(),
        nullptr,
        *AmbientSoundClassPath
    ));

    DialogueSoundClass = Cast<USoundClass>(StaticLoadObject(
        USoundClass::StaticClass(),
        nullptr,
        *DialogueSoundClassPath
    ));

    UISoundClass = Cast<USoundClass>(StaticLoadObject(
        USoundClass::StaticClass(),
        nullptr,
        *UISoundClassPath
    ));

    // Log results
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Sound assets loaded (Master: %s, Music: %s, SFX: %s, Ambient: %s, Dialogue: %s, UI: %s)"),
        MasterSoundClass ? TEXT("OK") : TEXT("FAIL"),
        MusicSoundClass ? TEXT("OK") : TEXT("FAIL"),
        SFXSoundClass ? TEXT("OK") : TEXT("FAIL"),
        AmbientSoundClass ? TEXT("OK") : TEXT("FAIL"),
        DialogueSoundClass ? TEXT("OK") : TEXT("FAIL"),
        UISoundClass ? TEXT("OK") : TEXT("FAIL"));
}

void FAudioSettingsHandler::ApplyVolumeToSoundClass(USoundClass* SoundClass, float Volume)
{
    if (!SoundClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioHandler: Cannot apply volume - SoundClass is null"));
        return;
    }

    // Method 1: Direct property modification
    SoundClass->Properties.Volume = Volume;

    // Method 2: Use SoundMix (more dynamic, runtime-friendly)
    // This requires the SoundMix to have the SoundClass configured
    // We'll push the mix in PushSoundMixModifier()

    UE_LOG(LogTemp, Verbose, TEXT("AudioHandler: Applied volume %.2f to SoundClass '%s'"),
        Volume, *SoundClass->GetName());
}

FAudioDevice* FAudioSettingsHandler::GetAudioDevice(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioHandler: World is null"));
        return nullptr;
    }

    FAudioDevice* AudioDevice = World->GetAudioDeviceRaw();

    if (!AudioDevice)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioHandler: Failed to get audio device from world"));
        return nullptr;
    }

    return AudioDevice;
}

void FAudioSettingsHandler::PushSoundMixModifier(UWorld* World)
{
    if (!World || !MasterSoundMix)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioHandler: Cannot push sound mix - World or MasterSoundMix is null"));
        return;
    }

    // Push the sound mix to apply it
    UGameplayStatics::PushSoundMixModifier(World, MasterSoundMix);

    // Set sound mix class overrides for each sound class
    if (MasterSoundClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(
            World,
            MasterSoundMix,
            MasterSoundClass,
            CachedMasterVolume,
            1.0f, // Pitch
            0.0f, // FadeInTime
            true  // bApplyToChildren
        );
    }

    if (MusicSoundClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(
            World,
            MasterSoundMix,
            MusicSoundClass,
            CachedMusicVolume,
            1.0f,
            0.0f,
            true
        );
    }

    if (SFXSoundClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(
            World,
            MasterSoundMix,
            SFXSoundClass,
            CachedSFXVolume,
            1.0f,
            0.0f,
            true
        );
    }

    if (AmbientSoundClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(
            World,
            MasterSoundMix,
            AmbientSoundClass,
            CachedAmbientVolume,
            1.0f,
            0.0f,
            true
        );
    }

    if (DialogueSoundClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(
            World,
            MasterSoundMix,
            DialogueSoundClass,
            CachedDialogueVolume,
            1.0f,
            0.0f,
            true
        );
    }

    if (UISoundClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(
            World,
            MasterSoundMix,
            UISoundClass,
            CachedUIVolume,
            1.0f,
            0.0f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Sound mix pushed and class overrides applied"));
}

void FAudioSettingsHandler::SetAudioOutputDevice(const FString& DeviceName)
{
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Audio output device set to '%s' (requires engine-level configuration)"),
        *DeviceName);

    // Note: Changing audio output device at runtime requires platform-specific code
    // and may not be fully supported in all Unreal Engine versions.
    // This typically needs to be set in Project Settings or via command line.
}

void FAudioSettingsHandler::SetSpatializationMethod(EE_AudioSpatialization Method)
{
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Spatialization method set to %d"),
        static_cast<int32>(Method));

    // Implementation requires modifying audio device settings
    // This is typically done through Project Settings -> Audio
}

void FAudioSettingsHandler::SetReverbQuality(EE_AudioQuality Quality)
{
    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Reverb quality set to %d"),
        static_cast<int32>(Quality));

    // Implementation requires adjusting reverb plugin settings
}

TArray<FString> FAudioSettingsHandler::GetAvailableAudioDevices()
{
    TArray<FString> Devices;

    // Note: Getting available audio devices requires platform-specific code
    // This is a simplified implementation

    Devices.Add(TEXT("Default"));
    Devices.Add(TEXT("Primary Sound Driver"));

    // On Windows, you might enumerate audio endpoints
    // On other platforms, the approach differs

    UE_LOG(LogTemp, Log, TEXT("AudioHandler: Found %d audio devices"), Devices.Num());

    return Devices;
}

void FAudioSettingsHandler::SetMuteAll(bool bMute)
{
    if (bMute)
    {
        CachedAudioSettings = AllSettings.AudioSettings;
        bWasMutedBySubsystem = true;
        SetMasterVolume(0.0f);
    }
    else
    {
        if (bWasMutedBySubsystem)
        {
            AllSettings.AudioSettings = CachedAudioSettings;
            //OnAudioSettingsChanged.Broadcast(AllSettings.AudioSettings);
            bDirtyFlag = true;
            bWasMutedBySubsystem = false;
        }
        else
        {
            // nobody muted via subsystem — chỉ đảm bảo master != 0
            if (AllSettings.AudioSettings.MasterVolume <= 0.0f)
            {
                SetMasterVolume(1.0f);
            }
        }
    }
}