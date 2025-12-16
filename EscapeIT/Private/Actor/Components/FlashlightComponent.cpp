
#include "Actor/Components/FlashlightComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/Item/Flashlight.h"
#include "TimerManager.h"
#include "Actor/Components/InventoryComponent.h"

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
    
    LastBatteryPercentage = GetBatteryPercentage();
    
    UE_LOG(LogTemp, Log, TEXT("FlashlightComponent: Initialized (Battery: %.1f%%)"), LastBatteryPercentage);
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

    // Validate SpotLight component
    if (!SpotLight)
    {
        UE_LOG(LogTemp, Error, TEXT("SetLightEnabled: SpotLight component is NULL!"));
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

    if (bIsLightOn)
    {
        // Turn ON: Enable everything
        SpotLight->SetVisibility(true);
        SpotLight->SetHiddenInGame(false);
        SpotLight->SetActive(true);
        
        // Set proper intensity
        UpdateLightIntensity();
        
        // Force update to make sure it's visible
        SpotLight->MarkRenderStateDirty();
    }
    else
    {
        // Turn OFF: Disable everything
        SpotLight->SetVisibility(false);
        SpotLight->SetHiddenInGame(true);
        SpotLight->SetActive(false);
    }

    // Play toggle sound
    PlayToggleSound();

    // Broadcast event
    OnFlashlightToggled.Broadcast(bIsLightOn);
    
    if (UTexture2D* NewIcon = UpdateFlashlightImage())
    {
        OnFlashlightImageChanged.Broadcast(NewIcon);
    }

    return true;
}

void UFlashlightComponent::EquipFlashlight(AFlashlight* FlashlightActor)
{
    if (bIsEquipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipFlashlight: Already equipped"));
        return;
    }

    if (!FlashlightActor)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipFlashlight: FlashlightActor is NULL!"));
        return;
    }

    // Store reference to flashlight actor
    CurrentFlashlightActor = FlashlightActor;

    // Get SpotLight component from the flashlight actor
    SpotLight = FlashlightActor->FindComponentByClass<USpotLightComponent>();
    
    if (SpotLight)
    {
        // **FIX: Initialize spotlight to OFF state properly**
        SpotLight->SetVisibility(false);
        SpotLight->SetHiddenInGame(true);
        SpotLight->SetActive(false);
        SpotLight->SetIntensity(NormalIntensity);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EquipFlashlight: No SpotLightComponent found on flashlight actor!"));
        return;
    }

    // Get character reference
    ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipFlashlight: No character found!"));
        return;
    }

    // Set equipped state
    bIsEquipped = true;
    bIsLightOn = false;
    
    OnFlashlightEquippedChanged.Broadcast(true);

    // Play equip animation if available
    if (EquipFlashlightAnim)
    {
        float AnimDuration = Character->PlayAnimMontage(EquipFlashlightAnim, 1.0f);
        
        if (AnimDuration > 0.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("EquipFlashlight: Playing equip animation (%.2fs)"), AnimDuration);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight: Equipped successfully (Battery: %.1f%%)"), GetBatteryPercentage());
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
    ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (Character && UnequipFlashlightAnim)
    {
        float AnimDuration = Character->PlayAnimMontage(UnequipFlashlightAnim, 1.0f);
        
        if (AnimDuration > 0.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("UnequipFlashlight: Playing unequip animation (%.2fs)"), AnimDuration);
        }
    }

    // Clear references
    SpotLight = nullptr;
    CurrentFlashlightActor = nullptr;
    
    // Set unequipped state
    bIsEquipped = false;
    bIsLightOn = false;
    
    OnFlashlightEquippedChanged.Broadcast(false);

    UE_LOG(LogTemp, Log, TEXT("Flashlight: Unequipped"));
}

// ============================================
// BATTERY MANAGEMENT
// ============================================

void UFlashlightComponent::ReplaceBattery()
{
    CurrentBattery = MaxBatteryDuration;
    LastBatteryPercentage = 100.0f;
    bLowBatterySoundPlayed = false;

    UpdateLightIntensity();
    PlaySound(BatteryReplaceSound);
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

    if (IsBatteryLow() && GetBatteryPercentage() > LowBatteryThreshold)
    {
        bLowBatterySoundPlayed = false;
    }

    if (bIsLightOn)
    {
        UpdateLightIntensity();
    }

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

    CurrentBattery -= DrainRate * DeltaTime;
    CurrentBattery = FMath::Max(CurrentBattery, 0.0f);

    float CurrentPercentage = GetBatteryPercentage();

    OnBatteryChanged.Broadcast(CurrentBattery, MaxBatteryDuration);

    if (!bLowBatterySoundPlayed && CurrentPercentage <= LowBatteryThreshold && LastBatteryPercentage > LowBatteryThreshold)
    {
        PlaySound(LowBatterySound);
        OnBatteryLow.Broadcast();
        bLowBatterySoundPlayed = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Battery: LOW! (%.1f%%)"), CurrentPercentage);
    }

    if (IsBatteryDepleted())
    {
        SetLightEnabled(false);
        OnBatteryDepleted.Broadcast();
        
        UE_LOG(LogTemp, Warning, TEXT("Battery: DEPLETED!"));
    }

    UpdateLightIntensity();

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

    if (IsBatteryLow())
    {
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

    FlickerTimer += DeltaTime * FlickerSpeed;
    float FlickerAmount = FMath::Sin(FlickerTimer) * FlickerIntensity * 0.5f + (1.0f - FlickerIntensity * 0.5f);

    float BaseIntensity = NormalIntensity;
    if (IsBatteryLow())
    {
        float Alpha = GetBatteryPercentage() / LowBatteryThreshold;
        BaseIntensity = FMath::Lerp(LowBatteryIntensity, NormalIntensity, Alpha);
    }

    float FlickeredIntensity = BaseIntensity * FlickerAmount;
    SpotLight->SetIntensity(FlickeredIntensity);
}

void UFlashlightComponent::HandleCriticalBattery()
{
    if (!SpotLight || !bIsLightOn)
    {
        return;
    }

    if (FMath::RandRange(0.0f, 1.0f) < 0.01f)
    {
        SpotLight->SetVisibility(false);
        SpotLight->SetHiddenInGame(true);

        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
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
            FMath::RandRange(0.95f, 1.05f)
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

void UFlashlightComponent::InitializeFlashlight(AFlashlight* OwnerFlashlight, USpotLightComponent* Spotlight)
{
    if (!OwnerFlashlight || !Spotlight)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeFlashlight: NULL parameters!"));
        return;
    }

    CurrentFlashlightActor = OwnerFlashlight;
    SpotLight = Spotlight;
    
    // Initialize spotlight to OFF state
    SpotLight->SetVisibility(false);
    SpotLight->SetHiddenInGame(true);
    SpotLight->SetActive(false);
    SpotLight->SetIntensity(NormalIntensity);
    
    // Make sure it's properly configured
    SpotLight->SetCastShadows(true);
    SpotLight->SetLightColor(FLinearColor::White);
    
    bIsEquipped = true;
    bIsLightOn = false;
    
    OnFlashlightEquippedChanged.Broadcast(true);
}

UTexture2D* UFlashlightComponent::UpdateFlashlightImage() const
{
    if (!CurrentFlashlightActor) return nullptr;

    if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this,0))
    {
        if (UInventoryComponent* InvComp = Character->FindComponentByClass<UInventoryComponent>())
        {
            FItemData ItemData;
            if (InvComp->GetEquippedItem(ItemData))
            {
                if (ItemData.ItemType == EItemType::Tool &&
                    ItemData.ToolType == EToolType::Flashlight)
                {
                    return !bIsLightOn ? ItemData.FlashlightOn : ItemData.FlashlightOff;
                }
            }
        }
    }
    return nullptr;
}

void UFlashlightComponent::SetEquipped(bool bEquipped)
{
    bool bWasEquipped = bIsEquipped;
    bIsEquipped = bEquipped;
    
    if (!bIsEquipped)
    {
        // Unequipping - turn off light if it's on
        if (bIsLightOn)
        {
            SetLightEnabled(false);
        }
        
        // Clear references
        SpotLight = nullptr;
        CurrentFlashlightActor = nullptr;
        
        UE_LOG(LogTemp, Log, TEXT("FlashlightComponent: UNEQUIPPED"));
    }
    else if (!bWasEquipped)
    {
        UE_LOG(LogTemp, Log, TEXT("FlashlightComponent: EQUIPPED"));
    }

    if (bWasEquipped != bIsEquipped)
    {
        OnFlashlightEquippedChanged.Broadcast(bIsEquipped);
    }
}