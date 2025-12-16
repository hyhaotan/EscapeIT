// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioManager.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Setup and Update
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetupAudioEffects();
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllAudio();

    // Trigger functions
    UFUNCTION(BlueprintCallable, Category = "Audio|JumpScare")
    void TriggerJumpScareAudio();
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Whispers")
    void TriggerWhisper();
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Static")
    void TriggerStaticNoiseSpike();
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Door")
    void PlayOpenDoorSound();
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Door")
    void PlayCloseDoorSound();
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound(USoundBase* Sound, float Volume = 1.0f);

    // Control functions
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetAmbientVolume(float NewVolume);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Breathing")
    void SetBreathingVolume(float NewVolume);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Heartbeat")
    void SetHeartbeatVolume(float NewVolume);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Heartbeat")
    void SetHeartbeatEnabled(bool bEnabled);

    // ============= Ambient Audio =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TObjectPtr<USoundBase> AmbientWindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bEnableAmbientAudio = true;

    // ============= Breathing Audio =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing")
    TObjectPtr<USoundBase> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BreathingVolume = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float BreathingSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing")
    bool bRandomBreathingPitch = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Breathing")
    bool bEnableBreathingAudio = true;

    // ============= Jump Scare Audio =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|JumpScare")
    TObjectPtr<USoundBase> JumpScareSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|JumpScare", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JumpScareVolume = 0.4f;

    // ============= Whisper System =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers")
    TArray<TObjectPtr<USoundBase>> RandomWhispers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WhisperVolume = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WhisperChance = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers", meta = (ClampMin = "0.0"))
    float WhisperMinInterval = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Whispers")
    bool bEnableWhispers = true;

    // ============= Static Noise =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Static")
    TObjectPtr<USoundBase> StaticNoiseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Static", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StaticNoiseSpikeChance = 0.01f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Static")
    bool bEnableStaticNoise = true;

    // ============= Heartbeat Audio =============
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat")
    TObjectPtr<USoundBase> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HeartbeatVolume = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Heartbeat")
    bool bEnableHeartbeatAudio = true;
    
    // ============== Door Audio ================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Door")
    TObjectPtr<USoundBase> OpenDoorSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Door")
    TObjectPtr<USoundBase> CloseDoorSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Door", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DoorVolume = 0.5f;

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> AmbientAudio;

    UPROPERTY()
    TObjectPtr<UAudioComponent> BreathingAudio;

    UPROPERTY()
    TObjectPtr<UAudioComponent> HeartbeatAudio;

    float LastWhisperTime = -999.0f;
    float TimeElapsed = 0.0f;

    void UpdateAudioEffects(float DeltaTime);
    void CheckForWhisper(float DeltaTime);
    void UpdateStaticNoise(float DeltaTime);
    void UpdateHeartbeatAudio(float DeltaTime);
    void UpdateBreathingAudio(float DeltaTime);
    void UpdateAmbientAudio(float DeltaTime);
    
    bool IsValidAudioComponent(UAudioComponent* AudioComp) const;
};