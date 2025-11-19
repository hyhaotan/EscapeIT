// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UAudioSubsystem::UAudioSubsystem()
{
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->bAutoActivate = false;

    BreathingAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("BreathingAudio"));
    BreathingAudio->bAutoActivate = false;

    HeartbeatAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("HeartbeatAudio"));
    HeartbeatAudio->bAutoActivate = false;
}

void UAudioSubsystem::PlaySound(USoundBase* Sound, float Volume)
{
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound, Volume);
    }
}

void UAudioSubsystem::SetupAudioEffects()
{
    // Ambient Wind
    if (AmbientAudio && AmbientWindSound)
    {
        AmbientAudio->SetSound(AmbientWindSound);
        AmbientAudio->SetVolumeMultiplier(AmbientVolume);
        AmbientAudio->Play();
    }

    // Breathing
    if (BreathingAudio && BreathingSound)
    {
        BreathingAudio->SetSound(BreathingSound);
        BreathingAudio->SetVolumeMultiplier(BreathingVolume);

        if (bRandomBreathingPitch)
        {
            float RandomPitch = FMath::RandRange(0.9f, 1.1f);
            BreathingAudio->SetPitchMultiplier(RandomPitch);
        }

        BreathingAudio->Play();
    }

    // Heartbeat
    if (HeartbeatAudio && HeartbeatSound && bEnableHeartbeatAudio)
    {
        HeartbeatAudio->SetSound(HeartbeatSound);
        HeartbeatAudio->SetVolumeMultiplier(HeartbeatVolume);
        HeartbeatAudio->Play();
    }
}

void UAudioSubsystem::UpdateAudioEffects(float DeltaTime, float TimeElapsed)
{
    // Breathing volume modulation
    if (BreathingAudio && BreathingAudio->IsPlaying())
    {
        float BreathCycle = FMath::Sin(TimeElapsed * BreathingSpeed);
        float VolumeModulation = FMath::Lerp(0.8f, 1.2f, (BreathCycle + 1.0f) * 0.5f);
        BreathingAudio->SetVolumeMultiplier(BreathingVolume * VolumeModulation);
    }

    // Ambient wind modulation (Perlin noise for natural variation)
    if (AmbientAudio && AmbientAudio->IsPlaying())
    {
        float AmbientCycle = FMath::PerlinNoise1D(TimeElapsed * 0.2f);
        float AmbientMod = FMath::Lerp(0.7f, 1.0f, (AmbientCycle + 1.0f) * 0.5f);
        AmbientAudio->SetVolumeMultiplier(AmbientVolume * AmbientMod);
    }

    // Check for whispers
    CheckForWhisper(DeltaTime, TimeElapsed);

    // Update static noise
    UpdateStaticNoise(DeltaTime);

    // Update heartbeat audio
    if (bEnableHeartbeatAudio)
    {
        UpdateHeartbeatAudio(DeltaTime, TimeElapsed);
    }
}

// ============= Jump Scare Audio =============
void UAudioSubsystem::TriggerJumpScareAudio()
{
    if (JumpScareSound)
    {
        UGameplayStatics::PlaySound2D(this, JumpScareSound, JumpScareVolume);
    }
}

// ============= Whisper System =============
void UAudioSubsystem::CheckForWhisper(float DeltaTime, float TimeElapsed)
{
    if (RandomWhispers.Num() == 0) return;
    if (TimeElapsed - LastWhisperTime < WhisperMinInterval) return;

    if (FMath::RandRange(0.0f, 1.0f) < WhisperChance * DeltaTime)
    {
        TriggerWhisper();
        LastWhisperTime = TimeElapsed;
    }
}

void UAudioSubsystem::TriggerWhisper()
{
    if (RandomWhispers.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, RandomWhispers.Num() - 1);
        USoundBase* WhisperSound = RandomWhispers[RandomIndex];

        if (WhisperSound)
        {
            // Random panning for spatial whisper effect
            float RandomPan = FMath::RandRange(-0.8f, 0.8f);
            UGameplayStatics::PlaySound2D(this, WhisperSound, WhisperVolume);
        }
    }
}

// ============= Static Noise =============
void UAudioSubsystem::UpdateStaticNoise(float DeltaTime)
{
    if (StaticNoiseSound)
    {
        if (FMath::RandRange(0.0f, 1.0f) < StaticNoiseSpikeChance * DeltaTime)
        {
            TriggerStaticNoiseSpike();
        }
    }
}

void UAudioSubsystem::TriggerStaticNoiseSpike()
{
    if (StaticNoiseSound)
    {
        float SpikeVolume = FMath::RandRange(0.1f, 0.3f);
        float RandomPitch = FMath::RandRange(0.8f, 1.2f);
        UGameplayStatics::PlaySound2D(this, StaticNoiseSound, SpikeVolume, RandomPitch);
    }
}

// ============= Heartbeat Audio =============
void UAudioSubsystem::UpdateHeartbeatAudio(float DeltaTime, float TimeElapsed)
{
    if (HeartbeatAudio && HeartbeatAudio->IsPlaying())
    {
        // Sync with heartbeat visual effect timing
        float HeartbeatCycle = FMath::Sin(TimeElapsed * 2.0f);
        float VolumeModulation = FMath::Lerp(0.5f, 1.0f, (HeartbeatCycle + 1.0f) * 0.5f);
        HeartbeatAudio->SetVolumeMultiplier(HeartbeatVolume * VolumeModulation);
    }
}