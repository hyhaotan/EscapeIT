// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LobbyCamera.generated.h"

class UCameraComponent;
class UPostProcessComponent;
class UAudioSubsystem;

UCLASS()
class ESCAPEIT_API ALobbyCamera : public APawn
{
    GENERATED_BODY()

public:
    ALobbyCamera();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

private:
    // ============= Components =============
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UPostProcessComponent* PostProcess;

    UPROPERTY()
    TObjectPtr<UAudioSubsystem> AudioSubsystem;

    // ============= Camera Movement Parameters =============
    UPROPERTY(EditAnywhere, Category = "Horror Effects|Camera Movement")
    float BreathingIntensity = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Camera Movement")
    float BreathingSpeed = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Camera Movement")
    float DriftSpeed = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Camera Movement")
    float DriftAmount = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Camera Movement")
    float FOVMin = 58.0f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Camera Movement")
    float FOVMax = 62.0f;

    // ============= Jump Scare Visual System =============
    UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
    float JumpScareChance = 0.03f; // 3% per second

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
    float JumpScareMinInterval = 15.0f; // Minimum 15s between scares

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
    float JumpScareIntensity = 8.0f; // Camera shake intensity

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
    float JumpScareDuration = 0.3f;

    // ============= Screen Glitch Effect =============
    UPROPERTY(EditAnywhere, Category = "Horror Effects|Glitch")
    float GlitchFrequency = 0.02f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Glitch")
    float GlitchDuration = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Glitch")
    float GlitchMaxChromaticAberration = 1.5f;

    // ============= Screen Distortion =============
    UPROPERTY(EditAnywhere, Category = "Horror Effects|Distortion")
    bool bEnableScreenDistortion = true;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Distortion")
    float DistortionFrequency = 0.05f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Distortion")
    float DistortionAmount = 0.02f;

    // ============= Heartbeat Visual Effect =============
    UPROPERTY(EditAnywhere, Category = "Horror Effects|Heartbeat")
    bool bEnableHeartbeat = true;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Heartbeat")
    float HeartbeatSpeed = 1.2f; // BPM factor

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Heartbeat")
    float HeartbeatVignetteIntensity = 0.3f;

    // ============= Shadow Flicker =============
    UPROPERTY(EditAnywhere, Category = "Horror Effects|Lighting")
    bool bEnableShadowFlicker = true;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Lighting")
    float ShadowFlickerChance = 0.01f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Lighting")
    float ShadowFlickerDuration = 0.2f;

    // ============= Post Process Settings =============
    UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
    float VignetteIntensity = 0.6f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
    float Saturation = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
    float Contrast = 1.2f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
    FLinearColor ColorGrading = FLinearColor(0.7f, 0.75f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
    float ChromaticAberration = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
    float FilmGrain = 0.4f;

    // ============= Runtime Variables =============
    FVector InitialLocation;
    FRotator InitialRotation;
    float InitialFOV;
    float TimeElapsed;

    // Runtime effect states
    float LastJumpScareTime;
    bool bIsGlitching;
    float GlitchTimer;
    bool bIsShadowFlickering;
    float ShadowFlickerTimer;
    bool bIsJumpScaring;
    float JumpScareTimer;
    
    // Cached base values for effects
    float BaseVignetteIntensity;
    float BaseChromaticAberration;

    // ============= Camera Movement Functions =============
    void UpdateCameraBreathing(float DeltaTime);
    void UpdateCameraDrift(float DeltaTime);
    void UpdateFOVBreathing(float DeltaTime);

    // ============= Visual Horror Effect Functions =============
    void CheckForJumpScare(float DeltaTime);
    void TriggerJumpScare();
    void UpdateJumpScare(float DeltaTime);

    void CheckForGlitch(float DeltaTime);
    void TriggerGlitch();
    void UpdateGlitch(float DeltaTime);

    void UpdateScreenDistortion(float DeltaTime);
    void UpdateHeartbeatEffect(float DeltaTime);

    void CheckForShadowFlicker(float DeltaTime);
    void TriggerShadowFlicker();
    void UpdateShadowFlicker(float DeltaTime);

    void SetupPostProcessEffects();
};