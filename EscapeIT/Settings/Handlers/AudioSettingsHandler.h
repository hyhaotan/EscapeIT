#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Data/SettingsTypes.h"

class ESCAPEIT_API FAudioSettingsHandler
{
public:
    static void ApplyToEngine(const FS_AudioSettings& Settings, UWorld* World);
    static void Initialize(UWorld* World);

    static void SetMasterVolume(float Volume);
    static void SetMusicVolume(float Volume);
    static void SetSFXVolume(float Volume);
    static void SetAmbientVolume(float Volume);
    static void SetDialogueVolume(float Volume);
    static void SetUIVolume(float Volume);

private:
    static class USoundMix* MasterSoundMix;
    static class USoundClass* MasterSoundClass;
    static class USoundClass* MusicSoundClass;
    static class USoundClass* SFXSoundClass;
    static class USoundClass* AmbientSoundClass;
    static class USoundClass* DialogueSoundClass;
    static class USoundClass* UISoundClass;

    static void LoadSoundAssets();
    static void ApplyVolumeToSoundClass(USoundClass* SoundClass, float Volume);
};