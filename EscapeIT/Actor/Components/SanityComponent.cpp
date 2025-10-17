
#include "SanityComponent.h"
#include "TimerManager.h"
#include "EscapeIT/EscapeITCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

USanityComponent::USanityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    MaxSanity = 100.0f;
    MinSanity = 0.0f;
    Sanity = MaxSanity;
    SanityDecayRate = 1.0f;
    bAutoDecay = false;

    RecoveryDelay = 3.0f;
    PassiveRecoveryRate = 2.0f;
    bIsInSafeZone = false;
    bIsRecovering = false;

    CurrentSanityLevel = ESanityLevel::High;
    PreviousSanityLevel = ESanityLevel::High;

    HighSanityThreshold = 70.0f;
    MediumSanityThreshold = 50.0f;
    LowSanityThreshold = 30.0f;

    DarknessDecayMultiplier = 2.0f;
    RecoveryMultiplier = 1.0f;

    VisualEffectIntensity = 0.0f;
    bIsInDarkZone = false;
    CurrentDecayMultiplier = 1.0f;

    HeartbeatShakeIntensity = 1.0f;
    DizzyRotationAmount = 3.0f;
    DizzyPulseSpeed = 1.0f;

    bIsCameraEffectActive = false;
    CameraEffectElapsedTime = 0.0f;
}

void USanityComponent::BeginPlay()
{
    Super::BeginPlay();

    Sanity = MaxSanity;
    CurrentSanityLevel = ESanityLevel::High;
    PreviousSanityLevel = ESanityLevel::High;

    OwnerCharacter = Cast<AEscapeITCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CameraComponent = OwnerCharacter->GetFirstPersonCameraComponent();

        APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->Controller);
        if (PlayerController)
        {
            PlayerCameraManager = PlayerController->PlayerCameraManager;
        }
    }
}

void USanityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Auto decay
    if (bAutoDecay && SanityDecayRate > 0.0f)
    {
        float DecayAmount = SanityDecayRate * CurrentDecayMultiplier * DeltaTime;
        CalculatorSanity(-DecayAmount);
    }

    // Passive recovery in safe zone only after RecoveryDelay has elapsed
    if (bIsInSafeZone && bIsRecovering && PassiveRecoveryRate > 0.0f)
    {
        float RecoveryAmount = PassiveRecoveryRate * RecoveryMultiplier * DeltaTime;
        CalculatorSanity(RecoveryAmount);
    }

    // Update visual effects
    UpdateVisualEffects();

    // Update camera panic effects
    UpdateCameraEffects();
}

// === GETTERS ===

float USanityComponent::GetSanity() const
{
    return Sanity;
}

float USanityComponent::GetMinSanity() const
{
    return MinSanity;
}

float USanityComponent::GetMaxSanity() const
{
    return MaxSanity;
}

float USanityComponent::GetSanityPercent() const
{
    if (MaxSanity <= MinSanity)
    {
        return 0.0f;
    }
    return (Sanity - MinSanity) / (MaxSanity - MinSanity);
}

ESanityLevel USanityComponent::GetSanityLevel() const
{
    return CurrentSanityLevel;
}

float USanityComponent::GetVisualEffectIntensity() const
{
    return VisualEffectIntensity;
}

// === SETTERS ===

void USanityComponent::SetSanity(float Amount)
{
    Sanity = FMath::Clamp(Amount, MinSanity, MaxSanity);
    UpdateSanity();
}

void USanityComponent::SetMinSanity(float Amount)
{
    MinSanity = Amount;
    Sanity = FMath::Clamp(Sanity, MinSanity, MaxSanity);
}

void USanityComponent::SetMaxSanity(float Amount)
{
    MaxSanity = Amount;
    Sanity = FMath::Clamp(Sanity, MinSanity, MaxSanity);
}

void USanityComponent::SetAutoDecay(bool bEnabled)
{
    bAutoDecay = bEnabled;
}

void USanityComponent::SetIsInSafeZone(bool bSafe)
{
    if (bSafe)
    {
        EnterSafeZone();
    }
    else
    {
        ExitSafeZone();
    }
}

void USanityComponent::SetRecoveryMultiplier(float Multiplier)
{
    RecoveryMultiplier = FMath::Max(0.0f, Multiplier);
}

// === CORE FUNCTIONS ===

void USanityComponent::ModifySanity(float Amount)
{
    CalculatorSanity(Amount);
}

void USanityComponent::RestoreSanity(float Amount)
{
    if (Amount > 0.0f)
    {
        CalculatorSanity(Amount);
    }
}

void USanityComponent::ReduceSanity(float Amount)
{
    if (Amount > 0.0f)
    {
        CalculatorSanity(-Amount);
    }
}

// === EVENT-BASED FUNCTIONS ===

void USanityComponent::ApplySanityEvent(const FSanityEventData& EventData)
{
    CalculatorSanity(EventData.Amount);
    OnSanityEvent.Broadcast(EventData.Amount, EventData.EventName);
}

void USanityComponent::OnWitnessHorror(float Amount)
{
    FSanityEventData EventData;
    EventData.Amount = -Amount;
    EventData.EventName = TEXT("Witnessed Horror");
    EventData.bShowNotification = true;
    ApplySanityEvent(EventData);
}

void USanityComponent::OnJumpScare(float Amount)
{
    FSanityEventData EventData;
    EventData.Amount = -Amount;
    EventData.EventName = TEXT("Jump Scare");
    EventData.bShowNotification = false;
    ApplySanityEvent(EventData);
}

void USanityComponent::OnPuzzleComplete(float Amount)
{
    FSanityEventData EventData;
    EventData.Amount = Amount;
    EventData.EventName = TEXT("Puzzle Completed");
    EventData.bShowNotification = true;
    ApplySanityEvent(EventData);
}

void USanityComponent::OnPuzzleFailed(float Amount)
{
    FSanityEventData EventData;
    EventData.Amount = -Amount;
    EventData.EventName = TEXT("Puzzle Failed");
    EventData.bShowNotification = false;
    ApplySanityEvent(EventData);
}

// === ZONE FUNCTIONS ===

void USanityComponent::EnterDarkZone()
{
    bIsInDarkZone = true;
    CurrentDecayMultiplier = DarknessDecayMultiplier;
}

void USanityComponent::ExitDarkZone()
{
    bIsInDarkZone = false;
    CurrentDecayMultiplier = 1.0f;
}

void USanityComponent::EnterSafeZone()
{
    bIsInSafeZone = true;
    bIsRecovering = false;

    if (RecoveryDelay > 0.0f)
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                RecoveryTimerHandle,
                this,
                &USanityComponent::StartRecovery,
                RecoveryDelay,
                false
            );
        }
    }
    else
    {
        // start recovery immediately
        StartRecovery();
    }
}

void USanityComponent::ExitSafeZone()
{
    bIsInSafeZone = false;
    bIsRecovering = false;

    if (RecoveryTimerHandle.IsValid() && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
    }
}

// === UTILITY ===

void USanityComponent::ResetSanity()
{
    SetSanity(MaxSanity);
}

bool USanityComponent::IsSanityDepleted() const
{
    return Sanity <= MinSanity;
}

bool USanityComponent::IsSanityCritical() const
{
    return CurrentSanityLevel == ESanityLevel::Critical;
}

void USanityComponent::CalculatorSanity(float Amount)
{
    float OldSanity = Sanity;
    Sanity = FMath::Clamp(Sanity + Amount, MinSanity, MaxSanity);

    if (!FMath::IsNearlyEqual(OldSanity, Sanity, 0.01f))
    {
        UpdateSanity();
    }
}

void USanityComponent::UpdateSanity()
{
    OnSanityChanged.Broadcast(Sanity);

    if (Sanity <= MinSanity)
    {
        OnSanityDepleted.Broadcast();
    }

    UpdateSanityLevel();
}

void USanityComponent::UpdateSanityLevel()
{
    ESanityLevel NewLevel;
    float Percent = GetSanityPercent() * 100.0f;

    if (Percent >= HighSanityThreshold)
    {
        NewLevel = ESanityLevel::High;
    }
    else if (Percent >= MediumSanityThreshold)
    {
        NewLevel = ESanityLevel::Medium;
    }
    else if (Percent >= LowSanityThreshold)
    {
        NewLevel = ESanityLevel::Low;
    }
    else
    {
        NewLevel = ESanityLevel::Critical;
    }

    if (NewLevel != PreviousSanityLevel)
    {
        PreviousSanityLevel = CurrentSanityLevel;
        CurrentSanityLevel = NewLevel;
        OnSanityLevelChanged.Broadcast(CurrentSanityLevel);
    }
}

void USanityComponent::UpdateVisualEffects()
{
    float Percent = GetSanityPercent();

    if (Percent >= 0.7f)
    {
        VisualEffectIntensity = 0.0f;
    }
    else if (Percent >= 0.5f)
    {
        VisualEffectIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.7f, 0.5f),
            FVector2D(0.0f, 0.3f),
            Percent
        );
    }
    else if (Percent >= 0.3f)
    {
        VisualEffectIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.5f, 0.3f),
            FVector2D(0.3f, 0.6f),
            Percent
        );
    }
    else
    {
        VisualEffectIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.3f, 0.0f),
            FVector2D(0.6f, 1.0f),
            Percent
        );
    }
}

void USanityComponent::StartRecovery()
{
    bIsRecovering = true;
}

FSanitySaveData USanityComponent::CaptureSaveData() const
{
    FSanitySaveData SaveData;

    SaveData.CurrentSanity = Sanity;
    SaveData.MaxSanity = MaxSanity;
    SaveData.MinSanity = MinSanity;
    SaveData.CurrentLevel = CurrentSanityLevel;
    SaveData.RecoveryMultiplier = RecoveryMultiplier;
    SaveData.bIsInSafeZone = bIsInSafeZone;
    SaveData.bIsInDarkZone = bIsInDarkZone;
    SaveData.bAutoDecay = bAutoDecay;
    SaveData.SanityDecayRate = SanityDecayRate;
    SaveData.SaveTime = FDateTime::Now();

    return SaveData;
}

void USanityComponent::LoadFromSaveData(const FSanitySaveData& SaveData)
{
    Sanity = SaveData.CurrentSanity;
    MaxSanity = SaveData.MaxSanity;
    MinSanity = SaveData.MinSanity;
    CurrentSanityLevel = SaveData.CurrentLevel;
    PreviousSanityLevel = SaveData.CurrentLevel;
    RecoveryMultiplier = SaveData.RecoveryMultiplier;
    bIsInSafeZone = SaveData.bIsInSafeZone;
    bIsInDarkZone = SaveData.bIsInDarkZone;
    bAutoDecay = SaveData.bAutoDecay;
    SanityDecayRate = SaveData.SanityDecayRate;

    UpdateSanity();
}

void USanityComponent::ResetToCheckpoint(const FSanitySaveData& CheckpointData)
{
    LoadFromSaveData(CheckpointData);
    OnSanityChanged.Broadcast(Sanity);
}

void USanityComponent::UpdateCameraEffects()
{
    if (!CameraComponent || !PlayerCameraManager)
    {
        return;
    }

    float Percent = GetSanityPercent();

    // Activate panic effect when sanity < 30%
    if (Percent < 0.3f && !bIsCameraEffectActive)
    {
        StartCameraPanicEffect();
    }
    else if (Percent >= 0.3f && bIsCameraEffectActive)
    {
        StopCameraPanicEffect();
    }

    if (bIsCameraEffectActive)
    {
        float DeltaTime = GetWorld()->DeltaTimeSeconds;

        ApplyCameraHeartbeatShake(DeltaTime);
        ApplyCameraDizzyEffect(DeltaTime);
        ApplyPanicPostProcess(Percent);

        CameraEffectElapsedTime += DeltaTime;
    }
}

void USanityComponent::StartCameraPanicEffect()
{
    bIsCameraEffectActive = true;
    CameraEffectElapsedTime = 0.0f;

    if (PlayerCameraManager)
    {
        if (HeartbeatCameraShake)
        {
            PlayerCameraManager->StartCameraShake(HeartbeatCameraShake, HeartbeatShakeIntensity * 2.0f);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HeartbeatCameraShake not assigned. Skipping initial shake class start."));
            // fallback: use StartCameraShake with null is not valid; just log.
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("🔴 PANIC MODE ACTIVATED - Vision deteriorating!"));
}

void USanityComponent::StopCameraPanicEffect()
{
    bIsCameraEffectActive = false;
    CameraEffectElapsedTime = 0.0f;

    if (CameraComponent)
    {
        FRotator CurrentRotation = CameraComponent->GetRelativeRotation();
        CameraComponent->SetRelativeRotation(FRotator(
            CurrentRotation.Pitch,
            CurrentRotation.Yaw,
            0.0f
        ));
    }

    if (PlayerCameraManager)
    {
        PlayerCameraManager->StopAllCameraShakes(true);
    }

    // Reset panic post process values by broadcasting zeroes
    OnPanicPostProcessUpdated.Broadcast(0.0f, 0.0f);

    UE_LOG(LogTemp, Warning, TEXT("🟢 PANIC MODE DEACTIVATED"));
}

void USanityComponent::ApplyCameraHeartbeatShake(float DeltaTime)
{
    if (!CameraComponent)
    {
        return;
    }

    float HeartbeatCycle = FMath::Fmod(CameraEffectElapsedTime, 1.0f);
    float ShakeIntensity = 0.0f;

    if (HeartbeatCycle < 0.2f)
    {
        float T = HeartbeatCycle / 0.2f;
        ShakeIntensity = HeartbeatShakeIntensity * FMath::Sin(T * PI);
    }
    else if (HeartbeatCycle < 0.4f)
    {
        ShakeIntensity = 0.0f;
    }
    else if (HeartbeatCycle < 0.55f)
    {
        float T = (HeartbeatCycle - 0.4f) / 0.15f;
        ShakeIntensity = HeartbeatShakeIntensity * 0.6f * FMath::Sin(T * PI);
    }
    else
    {
        ShakeIntensity = 0.0f;
    }

    float RandomShake = FMath::RandRange(-ShakeIntensity * 0.3f, ShakeIntensity * 0.3f);

    FVector ShakeDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-0.5f, 0.5f)
    ).GetSafeNormal();

    FVector CurrentLocation = CameraComponent->GetRelativeLocation();
    CameraComponent->SetRelativeLocation(CurrentLocation + ShakeDirection * (ShakeIntensity + RandomShake) * DeltaTime);
}

void USanityComponent::ApplyCameraDizzyEffect(float DeltaTime)
{
    if (!CameraComponent)
    {
        return;
    }

    float DizzyRotation = FMath::Sin(CameraEffectElapsedTime * DizzyPulseSpeed) * DizzyRotationAmount;
    DizzyRotation += FMath::Cos(CameraEffectElapsedTime * DizzyPulseSpeed * 1.3f) * DizzyRotationAmount * 0.5f;
    DizzyRotation += FMath::Sin(CameraEffectElapsedTime * 8.0f) * DizzyRotationAmount * 0.3f;

    FRotator CurrentRotation = CameraComponent->GetRelativeRotation();
    CameraComponent->SetRelativeRotation(FRotator(
        CurrentRotation.Pitch + FMath::Sin(CameraEffectElapsedTime * 1.5f) * 5.0f,
        CurrentRotation.Yaw,
        DizzyRotation
    ));
}

void USanityComponent::ApplyPanicPostProcess(float SanityPercent)
{
    if (!PlayerCameraManager)
    {
        return;
    }

    float PanicIntensity = 0.0f;

    if (SanityPercent < 0.3f)
    {
        PanicIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.3f, 0.0f),
            FVector2D(0.0f, 1.0f),
            SanityPercent
        );
    }

    ApplyVignetteEffect(SanityPercent);
    ApplyMotionBlur(SanityPercent);
}

void USanityComponent::ApplyVignetteEffect(float SanityPercent)
{
    float VignetteAmount = 0.0f;

    if (SanityPercent < 0.3f)
    {
        VignetteAmount = FMath::GetMappedRangeValueClamped(
            FVector2D(0.3f, 0.0f),
            FVector2D(0.0f, 0.8f),
            SanityPercent
        );
    }

    // Broadcast to any Blueprint listener to actually apply to post process / UI overlay
    OnPanicPostProcessUpdated.Broadcast(VignetteAmount, 0.0f);
}

void USanityComponent::ApplyMotionBlur(float SanityPercent)
{
    float BlurIntensity = 0.0f;

    if (SanityPercent < 0.3f)
    {
        BlurIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.3f, 0.0f),
            FVector2D(0.0f, 1.0f),
            SanityPercent
        );
    }

    // Broadcast motion blur in second param (vignette was in first param in ApplyVignetteEffect)
    OnPanicPostProcessUpdated.Broadcast(0.0f, BlurIntensity);
}
