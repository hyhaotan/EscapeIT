// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS(Blueprintable)
class UAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSubsystem();

    // Setup and Update
    void SetupAudioEffects();
    void UpdateAudioEffects(float DeltaTime, float TimeElapsed);

    // Trigger functions
    void TriggerJumpScareAudio();
    void TriggerWhisper();
    void TriggerStaticNoiseSpike();
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound(USoundBase* Sound, float Volume = 1.0f);

    // ============= Ambient Audio =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* AmbientWindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float AmbientVolume = 0.3f;

    // ============= Breathing Audio =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing")
    USoundBase* BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing")
    float BreathingVolume = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing")
    float BreathingSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing")
    bool bRandomBreathingPitch = true;

    // ============= Jump Scare Audio =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|JumpScare")
    USoundBase* JumpScareSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|JumpScare")
    float JumpScareVolume = 0.4f;

    // ============= Whisper System =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers")
    TArray<USoundBase*> RandomWhispers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers")
    float WhisperVolume = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers")
    float WhisperChance = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers")
    float WhisperMinInterval = 20.0f;

    // ============= Static Noise =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Static")
    USoundBase* StaticNoiseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Static")
    float StaticNoiseSpikeChance = 0.01f;

    // ============= Heartbeat Audio =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat")
    USoundBase* HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat")
    float HeartbeatVolume = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat")
    bool bEnableHeartbeatAudio = true;
    
    // ============== Door Audio ================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Door")
    USoundBase* OpenDoorSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Door")
    USoundBase* CloseDoorSound;
private:
    UPROPERTY()
    UAudioComponent* AmbientAudio;

    UPROPERTY()
    UAudioComponent* BreathingAudio;

    UPROPERTY()
    UAudioComponent* HeartbeatAudio;

    float LastWhisperTime = -999.0f;

    void CheckForWhisper(float DeltaTime, float TimeElapsed);
    void UpdateStaticNoise(float DeltaTime);
    void UpdateHeartbeatAudio(float DeltaTime, float TimeElapsed);
};