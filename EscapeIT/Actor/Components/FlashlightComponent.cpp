#include "FlashlightComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UFlashlightComponent::UFlashlightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize battery to max
    CurrentBattery = MaxBatteryDuration;
    LastBatteryPercentage = 100.0f;
}

void UFlashlightComponent::BeginPlay()
{
    Super::BeginPlay();

    // Try to find SpotLight component in owner
    if (AActor* Owner = GetOwner())
    {
        SpotLight = Owner->FindComponentByClass<USpotLightComponent>();
        
        if (SpotLight)
        {
            // Initialize spotlight to off state
            SpotLight->SetVisibility(false);
            SpotLight->SetIntensity(NormalIntensity);
            
            UE_LOG(LogTemp, Log, TEXT("FlashlightComponent: SpotLight found and initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FlashlightComponent: No SpotLightComponent found on owner!"));
        }
    }

    // Initialize battery percentage tracking
    LastBatteryPercentage = GetBatteryPercentage();
}

void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Only update if equipped and light is on
    if (bIsEquipped && bIsLightOn)
    {
        UpdateBattery(DeltaTime);

        // Apply visual effects based on battery level
        if (IsBatteryLow())
        {
            ApplyFlickerEffect(DeltaTime);
        }
    }
}

// ============================================
// TOGGLE & CONTROL
// ============================================

bool UFlashlightComponent::ToggleLight()
{
    return SetLightEnabled(!bIsLightOn);
}

bool UFlashlightComponent::SetLightEnabled(bool bEnabled)
{
    // Check if flashlight is equipped
    if (!bIsEquipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetLightEnabled: Flashlight not equipped!"));
        return false;
    }

    // Check if trying to turn on with no battery
    if (bEnabled && IsBatteryDepleted())
    {
        UE_LOG(LogTemp, Warning, TEXT("SetLightEnabled: Cannot turn on - Battery depleted!"));
        PlaySound(NoBatterySound);
        OnBatteryDepleted.Broadcast();
        return false;
    }

    // Update state
    bIsLightOn = bEnabled;

    // Update spotlight visibility
    if (SpotLight)
    {
        SpotLight->SetVisibility(bIsLightOn);
        
        // Reset intensity when turning on
        if (bIsLightOn)
        {
            UpdateLightIntensity();
        }

        UE_LOG(LogTemp, Log, TEXT("Flashlight Light: %s (Intensity: %.1f)"), 
            bIsLightOn ? TEXT("ON") : TEXT("OFF"), 
            SpotLight->Intensity);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SetLightEnabled: SpotLight is NULL!"));
        return false;
    }

    // Play toggle sound
    PlayToggleSound();

    // Broadcast event
    OnFlashlightToggled.Broadcast(bIsLightOn);

    return true;
}

void UFlashlightComponent::EquipFlashlight()
{
    if (bIsEquipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipFlashlight: Already equipped"));
        return;
    }

    // Get character reference
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        Character = UGameplayStatics::GetPlayerCharacter(this, 0);
    }

    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipFlashlight: No character found!"));
        return;
    }

    // Set equipped state
    bIsEquipped = true;

    // Play equip animation if available
    if (EquipFlashlightAnim)
    {
        float AnimDuration = Character->PlayAnimMontage(EquipFlashlightAnim, 1.0f);
        
        if (AnimDuration > 0.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("EquipFlashlight: Playing equip animation (%.2fs)"), AnimDuration);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("EquipFlashlight: Failed to play equip animation"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight: Equipped (Battery: %.1f%%)"), GetBatteryPercentage());
}

void UFlashlightComponent::UnequipFlashlight()
{
    if (!bIsEquipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnequipFlashlight: Not equipped"));
        return;
    }

    // Turn off light if it's on
    if (bIsLightOn)
    {
        SetLightEnabled(false);
    }

    // Get character reference
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        Character = UGameplayStatics::GetPlayerCharacter(this, 0);
    }

    if (Character)
    {
        // Play unequip animation if available
        if (UnequipFlashlightAnim)
        {
            float AnimDuration = Character->PlayAnimMontage(UnequipFlashlightAnim, 1.0f);
            
            if (AnimDuration > 0.0f)
            {
                UE_LOG(LogTemp, Log, TEXT("UnequipFlashlight: Playing unequip animation (%.2fs)"), AnimDuration);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UnequipFlashlight: Failed to play unequip animation"));
            }
        }
    }

    // Set unequipped state
    bIsEquipped = false;

    UE_LOG(LogTemp, Log, TEXT("Flashlight: Unequipped"));
}

// ============================================
// BATTERY MANAGEMENT
// ============================================

void UFlashlightComponent::ReplaceBattery()
{
    // Replace with new battery (100% charge)
    CurrentBattery = MaxBatteryDuration;
    LastBatteryPercentage = 100.0f;
    bLowBatterySoundPlayed = false;

    // Update light intensity
    UpdateLightIntensity();

    // Play replace sound
    PlaySound(BatteryReplaceSound);

    // Broadcast events
    OnBatteryChanged.Broadcast(CurrentBattery, MaxBatteryDuration);

    UE_LOG(LogTemp, Log, TEXT("Battery: Replaced (100%% - %.1fs duration)"), MaxBatteryDuration);
}

void UFlashlightComponent::AddBatteryCharge(float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }

    float OldBattery = CurrentBattery;
    CurrentBattery = FMath::Clamp(CurrentBattery + Amount, 0.0f, MaxBatteryDuration);

    // Check if crossed low battery threshold
    if (IsBatteryLow() && GetBatteryPercentage() > LowBatteryThreshold)
    {
        bLowBatterySoundPlayed = false;
    }

    // Update light intensity
    if (bIsLightOn)
    {
        UpdateLightIntensity();
    }

    // Broadcast event
    OnBatteryChanged.Broadcast(CurrentBattery, MaxBatteryDuration);

    UE_LOG(LogTemp, Log, TEXT("Battery: Added %.1fs charge (%.1f%% -> %.1f%%)"), 
        Amount, 
        (OldBattery / MaxBatteryDuration) * 100.0f,
        GetBatteryPercentage());
}

float UFlashlightComponent::GetBatteryPercentage() const
{
    if (MaxBatteryDuration <= 0.0f)
    {
        return 0.0f;
    }
    return (CurrentBattery / MaxBatteryDuration) * 100.0f;
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
    return bIsEquipped && !IsBatteryDepleted();
}

// ============================================
// INTERNAL FUNCTIONS - BATTERY
// ============================================

void UFlashlightComponent::UpdateBattery(float DeltaTime)
{
    if (CurrentBattery <= 0.0f)
    {
        return;
    }

    // Drain battery
    CurrentBattery -= DrainRate * DeltaTime;
    CurrentBattery = FMath::Max(CurrentBattery, 0.0f);

    float CurrentPercentage = GetBatteryPercentage();

    // Broadcast battery update
    OnBatteryChanged.Broadcast(CurrentBattery, MaxBatteryDuration);

    // Check for low battery warning (one-time)
    if (!bLowBatterySoundPlayed && CurrentPercentage <= LowBatteryThreshold && LastBatteryPercentage > LowBatteryThreshold)
    {
        PlaySound(LowBatterySound);
        OnBatteryLow.Broadcast();
        bLowBatterySoundPlayed = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Battery: LOW! (%.1f%%)"), CurrentPercentage);
    }

    // Check for battery depletion
    if (IsBatteryDepleted())
    {
        SetLightEnabled(false);
        OnBatteryDepleted.Broadcast();
        
        UE_LOG(LogTemp, Warning, TEXT("Battery: DEPLETED!"));
    }

    // Update light intensity based on battery level
    UpdateLightIntensity();

    // Handle critical battery moments
    if (CurrentPercentage < 5.0f)
    {
        HandleCriticalBattery();
    }

    LastBatteryPercentage = CurrentPercentage;
}

void UFlashlightComponent::UpdateLightIntensity()
{
    if (!SpotLight || !bIsLightOn)
    {
        return;
    }

    float TargetIntensity = NormalIntensity;
    float CurrentPercentage = GetBatteryPercentage();

    // Gradually reduce intensity when battery is low
    if (IsBatteryLow())
    {
        // Linear interpolation from LowBatteryIntensity to NormalIntensity
        float Alpha = CurrentPercentage / LowBatteryThreshold;
        TargetIntensity = FMath::Lerp(LowBatteryIntensity, NormalIntensity, Alpha);
    }

    SpotLight->SetIntensity(TargetIntensity);
}

// ============================================
// INTERNAL FUNCTIONS - VISUAL EFFECTS
// ============================================

void UFlashlightComponent::ApplyFlickerEffect(float DeltaTime)
{
    if (!SpotLight || !bIsLightOn)
    {
        return;
    }

    // Update flicker timer
    FlickerTimer += DeltaTime * FlickerSpeed;

    // Calculate flicker amount using sine wave
    // Range: (1.0 - FlickerIntensity) to 1.0
    float FlickerAmount = FMath::Sin(FlickerTimer) * FlickerIntensity * 0.5f + (1.0f - FlickerIntensity * 0.5f);

    // Get current target intensity
    float BaseIntensity = NormalIntensity;
    if (IsBatteryLow())
    {
        float Alpha = GetBatteryPercentage() / LowBatteryThreshold;
        BaseIntensity = FMath::Lerp(LowBatteryIntensity, NormalIntensity, Alpha);
    }

    // Apply flicker
    float FlickeredIntensity = BaseIntensity * FlickerAmount;
    SpotLight->SetIntensity(FlickeredIntensity);
}

void UFlashlightComponent::HandleCriticalBattery()
{
    if (!SpotLight || !bIsLightOn)
    {
        return;
    }

    // Random chance to cause momentary blackout (1% per frame)
    if (FMath::RandRange(0.0f, 1.0f) < 0.01f)
    {
        SpotLight->SetVisibility(false);

        // Schedule re-enable after short delay
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [this]()
            {
                if (SpotLight && bIsLightOn && !IsBatteryDepleted())
                {
                    SpotLight->SetVisibility(true);
                }
            },
            FMath::RandRange(0.05f, 0.15f), // 50-150ms blackout
            false
        );
    }
}

// ============================================
// INTERNAL FUNCTIONS - AUDIO
// ============================================

void UFlashlightComponent::PlaySound(USoundBase* Sound)
{
    if (Sound && GetOwner())
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            Sound,
            GetOwner()->GetActorLocation(),
            1.0f,
            FMath::RandRange(0.95f, 1.05f) // Slight pitch variation
        );
    }
}

void UFlashlightComponent::PlayToggleSound()
{
    if (bIsLightOn)
    {
        PlaySound(ToggleOnSound);
    }
    else
    {
        PlaySound(ToggleOffSound);
    }
}