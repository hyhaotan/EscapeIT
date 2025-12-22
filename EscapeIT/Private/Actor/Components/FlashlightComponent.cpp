
#include "Actor/Components/FlashlightComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/Item/Flashlight.h"
#include "TimerManager.h"
#include "Actor/Components/InventoryComponent.h"
#include "Components/TextBlock.h"

UFlashlightComponent::UFlashlightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize battery to max
    CurrentBattery = ItemData.BatteryDuration;
    LastBatteryPercentage = 100.0f;
    CurrentLightIntensity = 0.0f;
    TargetLightIntensity = 0.0f;
    
    // Initialize state
    CurrentState = EFlashlightState::Unequipped;
    bIsLightOn = false;
    bWasLightOnBeforeUnequip = false;
    bLowBatterySoundPlayed = false;
    bIsFadingLight = false;
}

void UFlashlightComponent::BeginPlay()
{
    Super::BeginPlay();
    
    LastBatteryPercentage = GetBatteryPercentage();
    
    UE_LOG(LogTemp, Log, TEXT("FlashlightComponent: Initialized (Battery: %.1f%%)"), LastBatteryPercentage);
}

void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Only process if equipped
    if (CurrentState != EFlashlightState::Equipped)
    {
        return;
    }

    // Update battery drain
    if (bIsLightOn && !IsBatteryDepleted())
    {
        UpdateBattery(DeltaTime);
    }

    // Update light intensity (smooth transitions + flicker)
    UpdateLightIntensity(DeltaTime);
}

// ============================================
// STATE MANAGEMENT
// ============================================

void UFlashlightComponent::SetState(EFlashlightState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    if (!ValidateStateTransition(NewState))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid state transition: %d -> %d"), 
            (int32)CurrentState, (int32)NewState);
        return;
    }

    EFlashlightState OldState = CurrentState;
    CurrentState = NewState;

    OnFlashlightStateChanged.Broadcast(NewState);

    UE_LOG(LogTemp, Log, TEXT("Flashlight state: %d -> %d"), (int32)OldState, (int32)NewState);
}

bool UFlashlightComponent::ValidateStateTransition(EFlashlightState NewState) const
{
    switch (CurrentState)
    {
    case EFlashlightState::Unequipped:
        return NewState == EFlashlightState::Equipping;

    case EFlashlightState::Equipping:
        return NewState == EFlashlightState::Equipped || NewState == EFlashlightState::Unequipped;

    case EFlashlightState::Equipped:
        return NewState == EFlashlightState::Unequipping;

    case EFlashlightState::Unequipping:
        return NewState == EFlashlightState::Unequipped || NewState == EFlashlightState::Equipped;

    default:
        return false;
    }
}

void UFlashlightComponent::OnEquipAnimationComplete()
{
    if (CurrentState == EFlashlightState::Equipping)
    {
        SetState(EFlashlightState::Equipped);
        
        // Restore previous light state if enabled
        if (bRememberLightState && bWasLightOnBeforeUnequip)
        {
            SetLightEnabled(true);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Flashlight: Equip animation complete"));
    }
}

void UFlashlightComponent::OnUnequipAnimationComplete()
{
    if (CurrentState == EFlashlightState::Unequipping)
    {
        SetState(EFlashlightState::Unequipped);
        CleanupFlashlight();
        
        UE_LOG(LogTemp, Log, TEXT("Flashlight: Unequip animation complete"));
    }
}

// ============================================
// PUBLIC API - TOGGLE & CONTROL
// ============================================

bool UFlashlightComponent::ToggleLight()
{
    return SetLightEnabled(!bIsLightOn);
}

bool UFlashlightComponent::SetLightEnabled(bool bEnabled)
{
    // Validate state
    if (CurrentState != EFlashlightState::Equipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot toggle light: Not equipped (State: %d)"), (int32)CurrentState);
        return false;
    }

    if (!SpotLight)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot toggle light: SpotLight is NULL"));
        return false;
    }

    // Check battery if trying to turn on
    if (bEnabled && IsBatteryDepleted())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot turn on: Battery depleted"));
        PlaySound(NoBatterySound);
        OnBatteryDepleted.Broadcast();
        return false;
    }

    // Already in desired state
    if (bIsLightOn == bEnabled)
    {
        return true;
    }

    // Change state
    bIsLightOn = bEnabled;

    // Set target intensity for smooth fade
    if (bIsLightOn)
    {
        TurnLightOn();
    }
    else
    {
        TurnLightOff();
    }

    // Play audio feedback
    PlayToggleSound();

    // Broadcast events
    OnFlashlightToggled.Broadcast(bIsLightOn);
    
    UTexture2D* NewIcon = GetFlashlightIcon();
    if (NewIcon)
    {
        OnFlashlightImageChanged.Broadcast(NewIcon);
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight: %s (Battery: %.1f%%)"),
        bIsLightOn ? TEXT("ON") : TEXT("OFF"),
        GetBatteryPercentage());

    return true;
}

bool UFlashlightComponent::EquipFlashlight(AFlashlight* FlashlightActor)
{
    // Validate state
    if (CurrentState != EFlashlightState::Unequipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipFlashlight: Already equipped or equipping"));
        return false;
    }

    if (!FlashlightActor)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipFlashlight: FlashlightActor is NULL"));
        return false;
    }

    // Store reference
    CurrentFlashlightActor = FlashlightActor;

    // Get SpotLight component
    SpotLight = FlashlightActor->FindComponentByClass<USpotLightComponent>();
    
    if (!SpotLight)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipFlashlight: No SpotLightComponent found"));
        CurrentFlashlightActor = nullptr;
        return false;
    }

    // Initialize spotlight in OFF state
    SpotLight->SetVisibility(false);
    SpotLight->SetHiddenInGame(true);
    SpotLight->SetActive(false);
    SpotLight->SetIntensity(0.0f);
    CurrentLightIntensity = 0.0f;
    TargetLightIntensity = 0.0f;

    // Change state to equipping
    SetState(EFlashlightState::Equipping);

    // Play equip animation if available
    if (EquipFlashlightAnim)
    {
        ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
        if (Character)
        {
            float AnimDuration = Character->PlayAnimMontage(EquipFlashlightAnim, 1.0f);
            
            if (AnimDuration > 0.0f)
            {
                // Set timer for animation complete
                GetWorld()->GetTimerManager().SetTimer(
                    EquipAnimationTimer,
                    this,
                    &UFlashlightComponent::OnEquipAnimationComplete,
                    AnimDuration,
                    false
                );
                
                UE_LOG(LogTemp, Log, TEXT("Flashlight: Playing equip animation (%.2fs)"), AnimDuration);
            }
            else
            {
                // Animation failed, complete immediately
                OnEquipAnimationComplete();
            }
        }
        else
        {
            OnEquipAnimationComplete();
        }
    }
    else
    {
        // No animation, complete immediately
        OnEquipAnimationComplete();
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight: Equipping... (Battery: %.1f%%)"), GetBatteryPercentage());
    return true;
}

void UFlashlightComponent::UnequipFlashlight()
{
    // Validate state
    if (CurrentState != EFlashlightState::Equipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnequipFlashlight: Not equipped"));
        return;
    }

    // Remember light state for re-equip
    if (bRememberLightState)
    {
        bWasLightOnBeforeUnequip = bIsLightOn;
    }

    // Turn off light if it's on
    if (bIsLightOn)
    {
        SetLightEnabled(false);
    }

    // Stop all timers
    StopLowBatteryBeep();

    // Change state to unequipping
    SetState(EFlashlightState::Unequipping);

    // Play unequip animation if available
    if (UnequipFlashlightAnim)
    {
        ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
        if (Character)
        {
            float AnimDuration = Character->PlayAnimMontage(UnequipFlashlightAnim, 1.0f);
            
            if (AnimDuration > 0.0f)
            {
                // Set timer for animation complete
                GetWorld()->GetTimerManager().SetTimer(
                    UnequipAnimationTimer,
                    this,
                    &UFlashlightComponent::OnUnequipAnimationComplete,
                    AnimDuration,
                    false
                );
                
                UE_LOG(LogTemp, Log, TEXT("Flashlight: Playing unequip animation (%.2fs)"), AnimDuration);
            }
            else
            {
                // Animation failed, complete immediately
                OnUnequipAnimationComplete();
            }
        }
        else
        {
            OnUnequipAnimationComplete();
        }
    }
    else
    {
        // No animation, complete immediately
        OnUnequipAnimationComplete();
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight: Unequipping..."));
}

// ============================================
// BATTERY MANAGEMENT
// ============================================

void UFlashlightComponent::AddBatteryCharge(float ChargePercent)
{
    if (ChargePercent <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid charge amount: %.1f%%"), ChargePercent);
        return;
    }

    float OldPercent = GetBatteryPercentage();
    
    // Convert percentage to seconds
    float ChargeSeconds = (ChargePercent / 100.0f) * ItemData.BatteryDuration;
    CurrentBattery = FMath::Clamp(CurrentBattery + ChargeSeconds, 0.0f, ItemData.BatteryDuration);

    float NewPercent = GetBatteryPercentage();
    float AddedPercent = NewPercent - OldPercent;

    // Reset low battery warning if charged above threshold
    if (OldPercent <= LowBatteryThreshold && NewPercent > LowBatteryThreshold)
    {
        bLowBatterySoundPlayed = false;
        StopLowBatteryBeep();
    }

    // Update light intensity if currently on
    if (bIsLightOn)
    {
        TargetLightIntensity = CalculateTargetIntensity();
    }

    // Broadcast event
    OnBatteryChanged.Broadcast(CurrentBattery, ItemData.BatteryDuration);

    // Play charge sound
    PlaySound(BatteryReplaceSound);

    UE_LOG(LogTemp, Log, TEXT("Battery: Charged +%.1f%% (%.1f%% → %.1f%%)"),
        AddedPercent, OldPercent, NewPercent);
}

void UFlashlightComponent::ReplaceBattery()
{
    CurrentBattery = ItemData.BatteryDuration;
    LastBatteryPercentage = 100.0f;
    bLowBatterySoundPlayed = false;

    StopLowBatteryBeep();
    
    if (bIsLightOn)
    {
        TargetLightIntensity = NormalIntensity;
    }

    PlaySound(BatteryReplaceSound);
    OnBatteryChanged.Broadcast(CurrentBattery, ItemData.BatteryDuration);

    UE_LOG(LogTemp, Log, TEXT("Battery: Replaced (100%%)"));
}

void UFlashlightComponent::UpdateBattery(float DeltaTime)
{
    if (CurrentBattery <= 0.0f)
    {
        return;
    }

    // Drain battery
    float DrainAmount = ItemData.BatteryDrainRate * DeltaTime;
    CurrentBattery = FMath::Max(CurrentBattery - DrainAmount, 0.0f);

    float CurrentPercentage = GetBatteryPercentage();

    // Broadcast battery change
    OnBatteryChanged.Broadcast(CurrentBattery, ItemData.BatteryDuration);

    // Check for low battery warning
    if (!bLowBatterySoundPlayed && CurrentPercentage <= LowBatteryThreshold && LastBatteryPercentage > LowBatteryThreshold)
    {
        HandleBatteryLow();
    }

    // Check for battery depletion
    if (IsBatteryDepleted())
    {
        HandleBatteryDepleted();
    }

    LastBatteryPercentage = CurrentPercentage;
}

void UFlashlightComponent::HandleBatteryDepleted()
{
    SetLightEnabled(false);
    OnBatteryDepleted.Broadcast();
    
    UE_LOG(LogTemp, Warning, TEXT("Battery: DEPLETED!"));
}

void UFlashlightComponent::HandleBatteryLow()
{
    PlaySound(LowBatterySound);
    OnBatteryLow.Broadcast();
    StartLowBatteryBeep();
    bLowBatterySoundPlayed = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Battery: LOW! (%.1f%%)"), GetBatteryPercentage());
}

// ============================================
// LIGHT CONTROL
// ============================================

void UFlashlightComponent::TurnLightOn()
{
    if (!SpotLight) return;

    // Enable visibility
    SpotLight->SetVisibility(true);
    SpotLight->SetHiddenInGame(false);
    SpotLight->SetActive(true);
    
    // Set target intensity for smooth fade-in
    TargetLightIntensity = CalculateTargetIntensity();
    bIsFadingLight = true;
    
    // Force render state update
    SpotLight->MarkRenderStateDirty();
    
    // Start low battery beep if needed
    if (IsBatteryLow())
    {
        StartLowBatteryBeep();
    }

    UE_LOG(LogTemp, Log, TEXT("Light: Turning ON (Target Intensity: %.1f)"), TargetLightIntensity);
}

void UFlashlightComponent::TurnLightOff()
{
    if (!SpotLight) return;

    // Set target intensity to 0 for smooth fade-out
    TargetLightIntensity = 0.0f;
    bIsFadingLight = true;
    
    // Stop low battery beep
    StopLowBatteryBeep();

    UE_LOG(LogTemp, Log, TEXT("Light: Turning OFF"));
}

float UFlashlightComponent::CalculateTargetIntensity() const
{
    if (!bIsLightOn)
    {
        return 0.0f;
    }

    float CurrentPercentage = GetBatteryPercentage();

    // Normal intensity when battery is above low threshold
    if (CurrentPercentage > LowBatteryThreshold)
    {
        return NormalIntensity;
    }

    // Reduced intensity when battery is low
    float Alpha = FMath::Clamp(CurrentPercentage / LowBatteryThreshold, 0.0f, 1.0f);
    return FMath::Lerp(LowBatteryIntensity, NormalIntensity, Alpha);
}

void UFlashlightComponent::UpdateLightIntensity(float DeltaTime)
{
    if (!SpotLight) return;

    // Update target intensity based on battery level
    if (bIsLightOn)
    {
        TargetLightIntensity = CalculateTargetIntensity();
    }

    // Smooth transition to target intensity
    if (!FMath::IsNearlyEqual(CurrentLightIntensity, TargetLightIntensity, 0.1f))
    {
        CurrentLightIntensity = FMath::FInterpTo(
            CurrentLightIntensity,
            TargetLightIntensity,
            DeltaTime,
            LightFadeSpeed
        );
        
        bIsFadingLight = true;
    }
    else
    {
        CurrentLightIntensity = TargetLightIntensity;
        
        // If faded to 0, disable the light completely
        if (CurrentLightIntensity <= 0.1f && !bIsLightOn)
        {
            SpotLight->SetVisibility(false);
            SpotLight->SetHiddenInGame(true);
            SpotLight->SetActive(false);
        }
        
        bIsFadingLight = false;
    }

    // Apply flicker effect if enabled and battery is low
    float FinalIntensity = CurrentLightIntensity;
    
    if (bEnableFlickerEffect && bIsLightOn && IsBatteryLow())
    {
        ApplyFlickerEffect(DeltaTime);
        
        // Calculate flicker
        float FlickerAmount = FMath::Sin(FlickerTimer) * FlickerIntensity * 0.5f + (1.0f - FlickerIntensity * 0.5f);
        FinalIntensity *= FlickerAmount;
    }

    // Handle critical battery effects
    if (bIsLightOn && GetBatteryPercentage() < CriticalBatteryThreshold)
    {
        HandleCriticalBattery();
    }

    SpotLight->SetIntensity(FinalIntensity);
}

void UFlashlightComponent::ApplyFlickerEffect(float DeltaTime)
{
    FlickerTimer += DeltaTime * FlickerSpeed;
}

void UFlashlightComponent::HandleCriticalBattery()
{
    if (!SpotLight || !bIsLightOn)
    {
        return;
    }

    // Random chance for temporary blackout
    if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per frame
    {
        // Temporary blackout
        SpotLight->SetVisibility(false);
        SpotLight->SetHiddenInGame(true);

        // Restore after short delay
        FTimerHandle TempTimer;
        GetWorld()->GetTimerManager().SetTimer(
            TempTimer,
            [this]()
            {
                if (SpotLight && bIsLightOn && !IsBatteryDepleted())
                {
                    SpotLight->SetVisibility(true);
                    SpotLight->SetHiddenInGame(false);
                }
            },
            FMath::RandRange(0.05f, 0.15f),
            false
        );
    }
}

// ============================================
// AUDIO
// ============================================

void UFlashlightComponent::PlaySound(USoundBase* Sound) const
{
    if (Sound && GetOwner() && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            Sound,
            GetOwner()->GetActorLocation(),
            1.0f,
            FMath::RandRange(0.95f, 1.05f)
        );
    }
}

void UFlashlightComponent::PlayToggleSound()
{
    USoundBase* SoundToPlay = bIsLightOn ? ToggleOnSound : ToggleOffSound;
    PlaySound(SoundToPlay);
}

void UFlashlightComponent::StartLowBatteryBeep()
{
    if (!GetWorld()) return;

    StopLowBatteryBeep();

    GetWorld()->GetTimerManager().SetTimer(
        LowBatteryBeepTimer,
        this,
        &UFlashlightComponent::LowBatteryBeep,
        LowBatteryBeepIntervalBase,
        false
    );
}

void UFlashlightComponent::LowBatteryBeep()
{
    if (!GetWorld()) return;

    PlaySound(LowBatteryBeepSound);

    float CurrentPercent = GetBatteryPercentage();

    // Stop beeping if battery is no longer low or depleted
    if (CurrentPercent > LowBatteryThreshold || IsBatteryDepleted())
    {
        StopLowBatteryBeep();
        return;
    }

    // Calculate next beep interval (faster as battery gets lower)
    float Threshold = FMath::Max(0.01f, LowBatteryThreshold);
    float NextInterval = FMath::Clamp(
        0.1f + (CurrentPercent / Threshold) * LowBatteryBeepIntervalBase,
        0.1f,
        LowBatteryBeepIntervalBase
    );

    // Schedule next beep
    GetWorld()->GetTimerManager().SetTimer(
        LowBatteryBeepTimer,
        this,
        &UFlashlightComponent::LowBatteryBeep,
        NextInterval,
        false
    );
}

void UFlashlightComponent::StopLowBatteryBeep()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(LowBatteryBeepTimer);
    }
}

// ============================================
// CLEANUP
// ============================================

void UFlashlightComponent::CleanupFlashlight()
{
    // Stop all timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(LowBatteryBeepTimer);
        GetWorld()->GetTimerManager().ClearTimer(EquipAnimationTimer);
        GetWorld()->GetTimerManager().ClearTimer(UnequipAnimationTimer);
    }

    // Clear references
    SpotLight = nullptr;
    CurrentFlashlightActor = nullptr;
    
    // Reset state
    bIsLightOn = false;
    CurrentLightIntensity = 0.0f;
    TargetLightIntensity = 0.0f;
    bIsFadingLight = false;
    
    CurrentState = EFlashlightState::Unequipped;
}

// ============================================
// GETTERS
// ============================================

float UFlashlightComponent::GetBatteryPercentage() const
{
    if (ItemData.BatteryDuration <= 0.0f)
    {
        return 0.0f;
    }
    return FMath::Clamp((CurrentBattery / ItemData.BatteryDuration) * 100.0f, 0.0f, 100.0f);
}

bool UFlashlightComponent::IsBatteryLow() const
{
    return GetBatteryPercentage() <= LowBatteryThreshold;
}

bool UFlashlightComponent::IsBatteryDepleted() const
{
    return CurrentBattery <= 0.0f;
}

bool UFlashlightComponent::CanToggleLight() const
{
    return (CurrentState == EFlashlightState::Equipped) && !IsBatteryDepleted();
}

UTexture2D* UFlashlightComponent::GetFlashlightIcon() const
{
    if (!CurrentFlashlightActor) return nullptr;

    ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Character) return nullptr;

    UInventoryComponent* InvComp = Character->FindComponentByClass<UInventoryComponent>();
    if (!InvComp) return nullptr;

    FItemData ItemDatas;
    if (!InvComp->GetEquippedItem(ItemDatas)) return nullptr;

    if (ItemDatas.ItemType != EItemType::Tool || ItemDatas.ToolType != EToolType::Flashlight)
    {
        return nullptr;
    }
    
    return !bIsLightOn ? ItemDatas.FlashlightOn : ItemDatas.FlashlightOff;
}