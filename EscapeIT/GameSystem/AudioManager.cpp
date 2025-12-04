// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UAudioManager::UAudioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Create audio components
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    if (AmbientAudio)
    {
        AmbientAudio->bAutoActivate = false;
    }

    BreathingAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("BreathingAudio"));
    if (BreathingAudio)
    {
        BreathingAudio->bAutoActivate = false;
    }

    HeartbeatAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("HeartbeatAudio"));
    if (HeartbeatAudio)
    {
        HeartbeatAudio->bAutoActivate = false;
    }
}

void UAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto setup audio when game starts
    SetupAudioEffects();
}

void UAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeElapsed += DeltaTime;
    UpdateAudioEffects(DeltaTime);
}

void UAudioManager::PlaySound(USoundBase* Sound, float Volume)
{
    if (Sound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound, Volume);
    }
}

void UAudioManager::SetupAudioEffects()
{
    // Ambient Wind
    if (bEnableAmbientAudio && IsValidAudioComponent(AmbientAudio) && AmbientWindSound)
    {
        AmbientAudio->SetSound(AmbientWindSound);
        AmbientAudio->SetVolumeMultiplier(AmbientVolume);
        AmbientAudio->Play();
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Ambient audio started"));
    }

    // Breathing
    if (bEnableBreathingAudio && IsValidAudioComponent(BreathingAudio) && BreathingSound)
    {
        BreathingAudio->SetSound(BreathingSound);
        BreathingAudio->SetVolumeMultiplier(BreathingVolume);

        if (bRandomBreathingPitch)
        {
            float RandomPitch = FMath::RandRange(0.9f, 1.1f);
            BreathingAudio->SetPitchMultiplier(RandomPitch);
        }

        BreathingAudio->Play();
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Breathing audio started"));
    }

    // Heartbeat
    if (bEnableHeartbeatAudio && IsValidAudioComponent(HeartbeatAudio) && HeartbeatSound)
    {
        HeartbeatAudio->SetSound(HeartbeatSound);
        HeartbeatAudio->SetVolumeMultiplier(HeartbeatVolume);
        HeartbeatAudio->Play();
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Heartbeat audio started"));
    }
}

void UAudioManager::StopAllAudio()
{
    if (IsValidAudioComponent(AmbientAudio))
    {
        AmbientAudio->Stop();
    }
    
    if (IsValidAudioComponent(BreathingAudio))
    {
        BreathingAudio->Stop();
    }
    
    if (IsValidAudioComponent(HeartbeatAudio))
    {
        HeartbeatAudio->Stop();
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: All audio stopped"));
}

void UAudioManager::UpdateAudioEffects(float DeltaTime)
{
    // Update individual audio systems
    UpdateBreathingAudio(DeltaTime);
    UpdateAmbientAudio(DeltaTime);
    
    if (bEnableWhispers)
    {
        CheckForWhisper(DeltaTime);
    }
    
    if (bEnableStaticNoise)
    {
        UpdateStaticNoise(DeltaTime);
    }
    
    if (bEnableHeartbeatAudio)
    {
        UpdateHeartbeatAudio(DeltaTime);
    }
}

void UAudioManager::UpdateBreathingAudio(float DeltaTime)
{
    if (!bEnableBreathingAudio || !IsValidAudioComponent(BreathingAudio)) return;
    
    if (BreathingAudio->IsPlaying())
    {
        float BreathCycle = FMath::Sin(TimeElapsed * BreathingSpeed);
        float VolumeModulation = FMath::Lerp(0.8f, 1.2f, (BreathCycle + 1.0f) * 0.5f);
        BreathingAudio->SetVolumeMultiplier(BreathingVolume * VolumeModulation);
    }
}

void UAudioManager::UpdateAmbientAudio(float DeltaTime)
{
    if (!bEnableAmbientAudio || !IsValidAudioComponent(AmbientAudio)) return;
    
    if (AmbientAudio->IsPlaying())
    {
        float AmbientCycle = FMath::PerlinNoise1D(TimeElapsed * 0.2f);
        float AmbientMod = FMath::Lerp(0.7f, 1.0f, (AmbientCycle + 1.0f) * 0.5f);
        AmbientAudio->SetVolumeMultiplier(AmbientVolume * AmbientMod);
    }
}

void UAudioManager::UpdateHeartbeatAudio(float DeltaTime)
{
    if (!IsValidAudioComponent(HeartbeatAudio)) return;
    
    if (HeartbeatAudio->IsPlaying())
    {
        float HeartbeatCycle = FMath::Sin(TimeElapsed * 2.0f);
        float VolumeModulation = FMath::Lerp(0.5f, 1.0f, (HeartbeatCycle + 1.0f) * 0.5f);
        HeartbeatAudio->SetVolumeMultiplier(HeartbeatVolume * VolumeModulation);
    }
}

// ============= Jump Scare Audio =============
void UAudioManager::TriggerJumpScareAudio()
{
    if (JumpScareSound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), JumpScareSound, JumpScareVolume);
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Jump scare triggered!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Jump scare sound is not assigned!"));
    }
}

// ============= Whisper System =============
void UAudioManager::CheckForWhisper(float DeltaTime)
{
    if (RandomWhispers.Num() == 0) return;
    if (TimeElapsed - LastWhisperTime < WhisperMinInterval) return;

    if (FMath::RandRange(0.0f, 1.0f) < WhisperChance * DeltaTime)
    {
        TriggerWhisper();
        LastWhisperTime = TimeElapsed;
    }
}

void UAudioManager::TriggerWhisper()
{
    if (RandomWhispers.Num() > 0 && GetWorld())
    {
        int32 RandomIndex = FMath::RandRange(0, RandomWhispers.Num() - 1);
        USoundBase* WhisperSound = RandomWhispers[RandomIndex];

        if (WhisperSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), WhisperSound, WhisperVolume);
            UE_LOG(LogTemp, Log, TEXT("AudioManager: Whisper triggered (Index: %d)"), RandomIndex);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: No whisper sounds assigned!"));
    }
}

// ============= Static Noise =============
void UAudioManager::UpdateStaticNoise(float DeltaTime)
{
    if (StaticNoiseSound && FMath::RandRange(0.0f, 1.0f) < StaticNoiseSpikeChance * DeltaTime)
    {
        TriggerStaticNoiseSpike();
    }
}

void UAudioManager::TriggerStaticNoiseSpike()
{
    if (StaticNoiseSound && GetWorld())
    {
        float SpikeVolume = FMath::RandRange(0.1f, 0.3f);
        float RandomPitch = FMath::RandRange(0.8f, 1.2f);
        UGameplayStatics::PlaySound2D(GetWorld(), StaticNoiseSound, SpikeVolume, RandomPitch);
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Static noise spike triggered"));
    }
}

// ============= Door ====================
void UAudioManager::PlayOpenDoorSound()
{
    if (OpenDoorSound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), OpenDoorSound, DoorVolume);
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Door opened"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Open door sound is not assigned!"));
    }
}

void UAudioManager::PlayCloseDoorSound()
{
    if (CloseDoorSound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), CloseDoorSound, DoorVolume);
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Door closed"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Close door sound is not assigned!"));
    }
}

// ============= Control Functions =============
void UAudioManager::SetAmbientVolume(float NewVolume)
{
    AmbientVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    if (IsValidAudioComponent(AmbientAudio))
    {
        AmbientAudio->SetVolumeMultiplier(AmbientVolume);
    }
}

void UAudioManager::SetBreathingVolume(float NewVolume)
{
    BreathingVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    if (IsValidAudioComponent(BreathingAudio))
    {
        BreathingAudio->SetVolumeMultiplier(BreathingVolume);
    }
}

void UAudioManager::SetHeartbeatVolume(float NewVolume)
{
    HeartbeatVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    if (IsValidAudioComponent(HeartbeatAudio))
    {
        HeartbeatAudio->SetVolumeMultiplier(HeartbeatVolume);
    }
}

void UAudioManager::SetHeartbeatEnabled(bool bEnabled)
{
    bEnableHeartbeatAudio = bEnabled;
    
    if (IsValidAudioComponent(HeartbeatAudio))
    {
        if (bEnabled && HeartbeatSound)
        {
            if (!HeartbeatAudio->IsPlaying())
            {
                HeartbeatAudio->Play();
            }
        }
        else
        {
            HeartbeatAudio->Stop();
        }
    }
}

// ============= Helper Functions =============
bool UAudioManager::IsValidAudioComponent(UAudioComponent* AudioComp) const
{
    return AudioComp != nullptr && AudioComp->IsValidLowLevel();
}