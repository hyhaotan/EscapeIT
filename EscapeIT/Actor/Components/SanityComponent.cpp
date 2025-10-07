#include "SanityComponent.h"
#include "TimerManager.h"

USanityComponent::USanityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    MaxSanity = 100.0f;
    MinSanity = 0.0f;
    Sanity = 100.0f;
    SanityDecayRate = 1.0f;
    bAutoDecay = false;

    RecoveryDelay = 3.0f;
    PassiveRecoveryRate = 2.0f;
    bIsInSafeZone = false;

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
}

void USanityComponent::BeginPlay()
{
    Super::BeginPlay();
    Sanity = MaxSanity;
    CurrentSanityLevel = ESanityLevel::High;
    PreviousSanityLevel = ESanityLevel::High;
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

    // Passive recovery trong safe zone
    if (bIsInSafeZone && PassiveRecoveryRate > 0.0f)
    {
        float RecoveryAmount = PassiveRecoveryRate * RecoveryMultiplier * DeltaTime;
        CalculatorSanity(RecoveryAmount);
    }

    // Update visual effects
    UpdateVisualEffects();
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
    bIsInSafeZone = bSafe;
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

    // Delay trước khi bắt đầu hồi
    if (RecoveryDelay > 0.0f)
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

void USanityComponent::ExitSafeZone()
{
    bIsInSafeZone = false;

    // Clear recovery timer
    if (RecoveryTimerHandle.IsValid())
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

// === PRIVATE FUNCTIONS ===

void USanityComponent::CalculatorSanity(float Amount)
{
    float OldSanity = Sanity;
    Sanity = FMath::Clamp(Sanity + Amount, MinSanity, MaxSanity);

    // Chỉ update nếu thực sự có thay đổi
    if (!FMath::IsNearlyEqual(OldSanity, Sanity, 0.01f))
    {
        UpdateSanity();
    }
}

void USanityComponent::UpdateSanity()
{
    OnSanityChanged.Broadcast(Sanity);

    // Check depletion
    if (Sanity <= MinSanity)
    {
        OnSanityDepleted.Broadcast();
    }

    // Update level
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

    // Broadcast nếu level thay đổi
    if (NewLevel != PreviousSanityLevel)
    {
        PreviousSanityLevel = CurrentSanityLevel;
        CurrentSanityLevel = NewLevel;
        OnSanityLevelChanged.Broadcast(CurrentSanityLevel);
    }
}

void USanityComponent::UpdateVisualEffects()
{
    // Tính intensity dựa trên Sanity level
    float Percent = GetSanityPercent();

    if (Percent >= 0.7f)
    {
        VisualEffectIntensity = 0.0f;
    }
    else if (Percent >= 0.5f)
    {
        // 70% -> 50%: intensity 0 -> 0.3
        VisualEffectIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.7f, 0.5f),
            FVector2D(0.0f, 0.3f),
            Percent
        );
    }
    else if (Percent >= 0.3f)
    {
        // 50% -> 30%: intensity 0.3 -> 0.6
        VisualEffectIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.5f, 0.3f),
            FVector2D(0.3f, 0.6f),
            Percent
        );
    }
    else
    {
        // < 30%: intensity 0.6 -> 1.0
        VisualEffectIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(0.3f, 0.0f),
            FVector2D(0.6f, 1.0f),
            Percent
        );
    }
}

void USanityComponent::StartRecovery()
{
    // Function này được gọi sau RecoveryDelay
    // Có thể thêm logic đặc biệt khi bắt đầu hồi phục
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

    // Update UI
    UpdateSanity();
}

void USanityComponent::ResetToCheckpoint(const FSanitySaveData& CheckpointData)
{
    LoadFromSaveData(CheckpointData);

    // Optional: Thêm hiệu ứng khi respawn
    OnSanityChanged.Broadcast(Sanity);
}