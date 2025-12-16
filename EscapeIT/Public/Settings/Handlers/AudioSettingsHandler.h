#pragma once

#include "CoreMinimal.h"
#include "Data/SettingsStructs.h" // FS_AudioSettings, FS_AllSettings
#include "Delegates/DelegateCombinations.h" // delegate types if needed

// Forward declarations
class USoundMix;
class USoundClass;
class UWorld;
class FAudioDevice;

class FAudioSettingsHandler
{
public:
    /** Initialize the audio handler and load sound assets */
    static void Initialize(UWorld* World);

    /** Apply audio settings to the engine (applies all fields of FS_AudioSettings) */
    static void ApplyToEngine(const FS_AudioSettings& Settings, UWorld* World);

    /** Individual volume setters */
    static void SetMasterVolume(float Volume);
    static void SetMusicVolume(float Volume);
    static void SetSFXVolume(float Volume);
    static void SetAmbientVolume(float Volume);
    static void SetDialogueVolume(float Volume);
    static void SetUIVolume(float Volume);

    /** Mute / unmute all audio (subsystem-level mute) */
    static void SetMuteAll(bool bMute);

    /** Audio output configuration */
    static void SetAudioOutputDevice(const FString& DeviceName);
    static void SetSpatializationMethod(EE_AudioSpatialization Method);
    static void SetReverbQuality(EE_AudioQuality Quality);

    /** Get available audio output devices */
    static TArray<FString> GetAvailableAudioDevices();

    /** Check if audio system is initialized */
    static bool IsInitialized() { return bIsInitialized; }

    /** Cleanup resources */
    static void Shutdown();

    /** Optional helpers: save / load settings (deferred to subsystem implementation) */
    // static void SaveSettings();
    // static void LoadSettings();

private:
    /** Load sound assets from content */
    static void LoadSoundAssets();

    /** Apply volume to a specific sound class (modifies the soundclass property) */
    static void ApplyVolumeToSoundClass(USoundClass* SoundClass, float Volume);

    /** Get the audio device for the world */
    static FAudioDevice* GetAudioDevice(UWorld* World);

    /** Create sound mix adjustments and push to engine */
    static void PushSoundMixModifier(UWorld* World);

private:
    /** Sound asset references */
    static USoundMix* MasterSoundMix;
    static USoundClass* MasterSoundClass;
    static USoundClass* MusicSoundClass;
    static USoundClass* SFXSoundClass;
    static USoundClass* AmbientSoundClass;
    static USoundClass* DialogueSoundClass;
    static USoundClass* UISoundClass;

    /** Cached volume values */
    static float CachedMasterVolume;
    static float CachedMusicVolume;
    static float CachedSFXVolume;
    static float CachedAmbientVolume;
    static float CachedDialogueVolume;
    static float CachedUIVolume;

    /** Initialization flag */
    static bool bIsInitialized;

    /** Sound class paths (can be configured) */
    static const FString MasterSoundClassPath;
    static const FString MusicSoundClassPath;
    static const FString SFXSoundClassPath;
    static const FString AmbientSoundClassPath;
    static const FString DialogueSoundClassPath;
    static const FString UISoundClassPath;
    static const FString MasterSoundMixPath;

    /** Cached settings (mutable so subsystem can store/restore) */
    static FS_AudioSettings CachedAudioSettings;
    static FS_AllSettings AllSettings;

    /** Mute bookkeeping and dirty flag for saving */
    static bool bWasMutedBySubsystem;
    static bool bDirtyFlag;
};
