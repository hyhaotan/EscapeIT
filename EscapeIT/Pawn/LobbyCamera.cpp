// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyCamera.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/KismetMathLibrary.h"

ALobbyCamera::ALobbyCamera()
{
    PrimaryActorTick.bCanEverTick = true;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    RootComponent = Camera;
    Camera->SetFieldOfView(60.0f);

    PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
    PostProcess->SetupAttachment(Camera);
    PostProcess->bUnbound = true;

    // Initialize variables
    TimeElapsed = 0.0f;
    InitialFOV = 60.0f;
    LastJumpScareTime = -999.0f;
    bIsGlitching = false;
    GlitchTimer = 0.0f;
    bIsShadowFlickering = false;
    ShadowFlickerTimer = 0.0f;
    bIsJumpScaring = false;
    JumpScareTimer = 0.0f;
}

void ALobbyCamera::BeginPlay()
{
    Super::BeginPlay();
    
    InitialLocation = GetActorLocation();
    InitialRotation = GetActorRotation();
    InitialFOV = Camera->FieldOfView;

    TimeElapsed = FMath::RandRange(0.0f, 100.0f);

    SetupPostProcessEffects();

    // Store base values for effects
    BaseVignetteIntensity = VignetteIntensity;
    BaseChromaticAberration = ChromaticAberration;
}

void ALobbyCamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeElapsed += DeltaTime;

    // Camera movement effects
    UpdateCameraBreathing(DeltaTime);
    UpdateCameraDrift(DeltaTime);
    UpdateFOVBreathing(DeltaTime);

    // Visual horror effects
    CheckForJumpScare(DeltaTime);
    UpdateJumpScare(DeltaTime);

    CheckForGlitch(DeltaTime);
    UpdateGlitch(DeltaTime);

    if (bEnableScreenDistortion)
    {
        UpdateScreenDistortion(DeltaTime);
    }

    if (bEnableHeartbeat)
    {
        UpdateHeartbeatEffect(DeltaTime);
    }

    CheckForShadowFlicker(DeltaTime);
    UpdateShadowFlicker(DeltaTime);
}

// ============= Camera Movement Effects =============
void ALobbyCamera::UpdateCameraBreathing(float DeltaTime)
{
    float BreathCycle = FMath::Sin(TimeElapsed * BreathingSpeed);

    FVector Offset = FVector(
        BreathCycle * BreathingIntensity * 0.5f,
        0.0f,
        BreathCycle * BreathingIntensity
    );

    FVector NewLocation = InitialLocation + Offset;
    SetActorLocation(NewLocation);
}

void ALobbyCamera::UpdateCameraDrift(float DeltaTime)
{
    float DriftX = FMath::Sin(TimeElapsed * DriftSpeed) * DriftAmount;
    float DriftY = FMath::Cos(TimeElapsed * DriftSpeed * 0.7f) * DriftAmount * 0.5f;

    FRotator NewRotation = InitialRotation;
    NewRotation.Yaw += DriftX;
    NewRotation.Pitch += DriftY;

    FRotator CurrentRotation = GetActorRotation();
    FRotator InterpolatedRotation = UKismetMathLibrary::RInterpTo(
        CurrentRotation,
        NewRotation,
        DeltaTime,
        2.0f
    );

    SetActorRotation(InterpolatedRotation);
}

void ALobbyCamera::UpdateFOVBreathing(float DeltaTime)
{
    float FOVCycle = FMath::Sin(TimeElapsed * BreathingSpeed * 0.5f);
    float TargetFOV = FMath::Lerp(FOVMin, FOVMax, (FOVCycle + 1.0f) * 0.5f);

    float CurrentFOV = Camera->FieldOfView;
    float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 1.0f);

    Camera->SetFieldOfView(NewFOV);
}

// ============= Jump Scare System (Visual) =============
void ALobbyCamera::CheckForJumpScare(float DeltaTime)
{
    if (bIsJumpScaring) return;

    if (TimeElapsed - LastJumpScareTime < JumpScareMinInterval) return;

    if (FMath::RandRange(0.0f, 1.0f) < JumpScareChance * DeltaTime)
    {
        TriggerJumpScare();
    }
}

void ALobbyCamera::TriggerJumpScare()
{
    bIsJumpScaring = true;
    JumpScareTimer = 0.0f;
    LastJumpScareTime = TimeElapsed;

    // Sudden FOV change
    Camera->SetFieldOfView(InitialFOV - 15.0f);

    // Camera shake
    FVector ShakeOffset = FVector(
        FMath::RandRange(-JumpScareIntensity, JumpScareIntensity),
        FMath::RandRange(-JumpScareIntensity, JumpScareIntensity),
        FMath::RandRange(-JumpScareIntensity * 0.5f, JumpScareIntensity * 0.5f)
    );
    SetActorLocation(InitialLocation + ShakeOffset);

    // Spike vignette
    if (PostProcess)
    {
        PostProcess->Settings.VignetteIntensity = BaseVignetteIntensity + 0.4f;
    }
}

void ALobbyCamera::UpdateJumpScare(float DeltaTime)
{
    if (!bIsJumpScaring) return;

    JumpScareTimer += DeltaTime;

    if (JumpScareTimer >= JumpScareDuration)
    {
        bIsJumpScaring = false;

        // Reset vignette
        if (PostProcess)
        {
            PostProcess->Settings.VignetteIntensity = BaseVignetteIntensity;
        }
    }
}

// ============= Screen Glitch Effect =============
void ALobbyCamera::CheckForGlitch(float DeltaTime)
{
    if (bIsGlitching) return;

    if (FMath::RandRange(0.0f, 1.0f) < GlitchFrequency * DeltaTime)
    {
        TriggerGlitch();
    }
}

void ALobbyCamera::TriggerGlitch()
{
    bIsGlitching = true;
    GlitchTimer = 0.0f;

    if (PostProcess)
    {
        // Spike chromatic aberration
        PostProcess->Settings.SceneFringeIntensity = FMath::RandRange(
            GlitchMaxChromaticAberration * 0.5f,
            GlitchMaxChromaticAberration
        );

        // Random saturation spike
        float GlitchSat = FMath::RandRange(0.0f, 0.5f);
        PostProcess->Settings.ColorSaturation = FVector4(GlitchSat, GlitchSat, GlitchSat, 1.0f);
    }
}

void ALobbyCamera::UpdateGlitch(float DeltaTime)
{
    if (!bIsGlitching) return;

    GlitchTimer += DeltaTime;

    if (GlitchTimer >= GlitchDuration)
    {
        bIsGlitching = false;

        // Reset effects
        if (PostProcess)
        {
            PostProcess->Settings.SceneFringeIntensity = BaseChromaticAberration;
            PostProcess->Settings.ColorSaturation = FVector4(Saturation, Saturation, Saturation, 1.0f);
        }
    }
}

// ============= Screen Distortion =============
void ALobbyCamera::UpdateScreenDistortion(float DeltaTime)
{
    if (FMath::RandRange(0.0f, 1.0f) < DistortionFrequency * DeltaTime)
    {
        // Subtle screen tear effect through camera rotation
        FRotator CurrentRot = GetActorRotation();
        CurrentRot.Roll += FMath::RandRange(-DistortionAmount, DistortionAmount);
        SetActorRotation(CurrentRot);
    }
}

// ============= Heartbeat Visual Effect =============
void ALobbyCamera::UpdateHeartbeatEffect(float DeltaTime)
{
    // Double-beat pattern like a real heartbeat
    float HeartbeatCycle = FMath::Sin(TimeElapsed * HeartbeatSpeed * 2.0f);
    float SecondBeat = FMath::Sin(TimeElapsed * HeartbeatSpeed * 2.0f + 0.5f);

    float CombinedBeat = FMath::Max(HeartbeatCycle, SecondBeat * 0.5f);

    if (CombinedBeat > 0.8f && PostProcess)
    {
        float PulseIntensity = FMath::Clamp((CombinedBeat - 0.8f) * 5.0f, 0.0f, 1.0f);
        float VignettePulse = BaseVignetteIntensity + (HeartbeatVignetteIntensity * PulseIntensity);
        PostProcess->Settings.VignetteIntensity = VignettePulse;
    }
}

// ============= Shadow Flicker =============
void ALobbyCamera::CheckForShadowFlicker(float DeltaTime)
{
    if (bIsShadowFlickering || !bEnableShadowFlicker) return;

    if (FMath::RandRange(0.0f, 1.0f) < ShadowFlickerChance * DeltaTime)
    {
        TriggerShadowFlicker();
    }
}

void ALobbyCamera::TriggerShadowFlicker()
{
    bIsShadowFlickering = true;
    ShadowFlickerTimer = 0.0f;

    if (PostProcess)
    {
        // Darken suddenly
        PostProcess->Settings.AutoExposureBias = -1.5f;
    }
}

void ALobbyCamera::UpdateShadowFlicker(float DeltaTime)
{
    if (!bIsShadowFlickering) return;

    ShadowFlickerTimer += DeltaTime;

    if (ShadowFlickerTimer >= ShadowFlickerDuration)
    {
        bIsShadowFlickering = false;

        if (PostProcess)
        {
            PostProcess->Settings.AutoExposureBias = -0.5f;
        }
    }
}

// ============= Post Process Setup =============
void ALobbyCamera::SetupPostProcessEffects()
{
    if (PostProcess)
    {
        PostProcess->Settings.bOverride_VignetteIntensity = true;
        PostProcess->Settings.VignetteIntensity = VignetteIntensity;

        PostProcess->Settings.bOverride_ColorSaturation = true;
        PostProcess->Settings.ColorSaturation = FVector4(Saturation, Saturation, Saturation, 1.0f);

        PostProcess->Settings.bOverride_ColorContrast = true;
        PostProcess->Settings.ColorContrast = FVector4(Contrast, Contrast, Contrast, 1.0f);

        PostProcess->Settings.bOverride_ColorGamma = true;
        PostProcess->Settings.ColorGamma = FVector4(ColorGrading.R, ColorGrading.G, ColorGrading.B, 1.0f);

        PostProcess->Settings.bOverride_SceneFringeIntensity = true;
        PostProcess->Settings.SceneFringeIntensity = ChromaticAberration;

        PostProcess->Settings.bOverride_FilmGrainIntensity = true;
        PostProcess->Settings.FilmGrainIntensity = FilmGrain;

        PostProcess->Settings.bOverride_AmbientOcclusionIntensity = true;
        PostProcess->Settings.AmbientOcclusionIntensity = 0.8f;

        PostProcess->Settings.bOverride_AutoExposureBias = true;
        PostProcess->Settings.AutoExposureBias = -0.5f;
    }
}