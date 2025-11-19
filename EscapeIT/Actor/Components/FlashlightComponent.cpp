#include "FlashlightComponent.h"
#include "Components/SpotLightComponent.h"
#include "EscapeIT/Actor/Item/Flashlight.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UFlashlightComponent::UFlashlightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentBattery = MaxBatteryDuration;
    
}

void UFlashlightComponent::BeginPlay()
{
    Super::BeginPlay();

    // Tìm SpotLight component trong owner
    AFlashlight* Owner = Cast<AFlashlight>(GetOwner());
    if (Owner)
    {
        SpotLight = Owner->FindComponentByClass<USpotLightComponent>();
        if (SpotLight)
        {
            SpotLight->SetVisibility(false);
            SpotLight->SetIntensity(NormalIntensity);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FlashlightComponent: No SpotLightComponent found on owner!"));
        }
    }
}

void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsEquipped && bIsLightOn)
    {
        UpdateBattery(DeltaTime);

        if (IsBatteryLow())
        {
            ApplyFlickerEffect(DeltaTime);
        }
    }
}

// ============================================
// TOGGLE & CONTROL
// ============================================
void UFlashlightComponent::ToggleLight()
{
    SetLightEnabled(!bIsLightOn);
}

void UFlashlightComponent::SetLightEnabled(bool bEnabled)
{
    if (!bIsEquipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetLightEnabled: Flashlight not equipped!"));
        return;
    }

    if (bEnabled && IsBatteryDepleted())
    {
        UE_LOG(LogTemp, Warning, TEXT("SetLightEnabled: Battery depleted!"));
        OnBatteryDepleted.Broadcast();
        return;
    }

    bIsLightOn = bEnabled;

    if (SpotLight)
    {
        SpotLight->SetVisibility(bIsLightOn);
        float CurrentIntensity = SpotLight->Intensity;
        UE_LOG(LogTemp, Warning, TEXT(">>> SUCCESS: Light %s, Intensity: %f"), 
            bIsLightOn ? TEXT("ON") : TEXT("OFF"), CurrentIntensity);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT(">>> FAILED: SpotLight is NULL!"));
    }

    // Play sound
    PlaySound(bIsLightOn ? ToggleOnSound : ToggleOffSound);

    // Broadcast
    OnFlashlightToggled.Broadcast(bIsLightOn);

    UE_LOG(LogTemp, Log, TEXT("Flashlight: %s"), bIsLightOn ? TEXT("ON") : TEXT("OFF"));
}

void UFlashlightComponent::EquipFlashlight()
{
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char)
    {
        // fallback: try GetPlayerCharacter
        Char = UGameplayStatics::GetPlayerCharacter(this, 0);
    }

    if (!bIsEquipped && Char)
    {
        if (EquipFlashlightAnim == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("EquipFlashlightAnim is null"));
        }

        bIsEquipped = true;
        
        if (EquipFlashlightAnim)
        {
            float Played = Char->PlayAnimMontage(EquipFlashlightAnim, 1.0f);
            if (Played <= 0.f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to play EquipFlashlightAnim"));
            }
        }

        UE_LOG(LogTemp, Log, TEXT("Flashlight: Equipped"));
    }
}

void UFlashlightComponent::UnequipFlashlight()
{
    if (bIsLightOn)
    {
        SetLightEnabled(false);
    }

    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char)
    {
        Char = UGameplayStatics::GetPlayerCharacter(this, 0);
    }

    if (bIsEquipped && Char)
    {
        if (UnEquipFlashlightAnim == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("UnEquipFlashlightAnim is null"));
        }

        bIsEquipped = false;

        if (UnEquipFlashlightAnim)
        {
            float Played = Char->PlayAnimMontage(UnEquipFlashlightAnim, 1.0f);
            if (Played <= 0.f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to play UnEquipFlashlightAnim"));
            }
        }

        UE_LOG(LogTemp, Log, TEXT("Flashlight: Unequipped"));
    }
}


// ============================================
// BATTERY MANAGEMENT
// ============================================
void UFlashlightComponent::ReplaceBattery()
{
    CurrentBattery = MaxBatteryDuration;
    bLowBatterySoundPlayed = false;

    UpdateLightIntensity();
    PlaySound(BatteryReplaceSound);

    OnBatteryChanged.Broadcast(GetBatteryPercentage());

    UE_LOG(LogTemp, Log, TEXT("Battery: Replaced (100%%)"));
}

void UFlashlightComponent::AddBatteryCharge(float Amount)
{
    CurrentBattery = FMath::Clamp(CurrentBattery + Amount, 0.0f, MaxBatteryDuration);
    OnBatteryChanged.Broadcast(GetBatteryPercentage());
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

// ============================================
// INTERNAL FUNCTIONS
// ============================================
void UFlashlightComponent::UpdateBattery(float DeltaTime)
{
    if (CurrentBattery > 0.0f)
    {
        CurrentBattery -= DrainRate * DeltaTime;
        CurrentBattery = FMath::Max(CurrentBattery, 0.0f);

        // Broadcast update
        OnBatteryChanged.Broadcast(GetBatteryPercentage());

        // Check low battery sound
        if (IsBatteryLow() && !bLowBatterySoundPlayed)
        {
            PlaySound(LowBatterySound);
            bLowBatterySoundPlayed = true;
        }

        // Check depleted
        if (IsBatteryDepleted())
        {
            SetLightEnabled(false);
            OnBatteryDepleted.Broadcast();
            UE_LOG(LogTemp, Warning, TEXT("Battery: Depleted!"));
        }
    }

    UpdateLightIntensity();
}

void UFlashlightComponent::UpdateLightIntensity()
{
    if (!SpotLight)
    {
        return;
    }

    float TargetIntensity = NormalIntensity;

    // Giảm intensity khi battery thấp
    if (IsBatteryLow())
    {
        float Percentage = GetBatteryPercentage() / LowBatteryThreshold;
        TargetIntensity = FMath::Lerp(LowBatteryIntensity, NormalIntensity, Percentage);
    }

    SpotLight->SetIntensity(TargetIntensity);
}

void UFlashlightComponent::ApplyFlickerEffect(float DeltaTime)
{
    if (!SpotLight)
    {
        return;
    }

    FlickerTimer += DeltaTime * FlickerSpeed;

    // Random flicker
    float FlickerAmount = FMath::Sin(FlickerTimer) * 0.3f + 0.7f; // 0.7 - 1.0
    float CurrentIntensity = SpotLight->Intensity;
    float FlickeredIntensity = CurrentIntensity * FlickerAmount;

    SpotLight->SetIntensity(FlickeredIntensity);

    // Random tắt thoáng qua khi battery rất thấp
    if (GetBatteryPercentage() < 5.0f)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance mỗi frame
        {
            SpotLight->SetVisibility(false);

            // Bật lại sau 0.1s
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
                {
                    if (SpotLight && bIsLightOn)
                    {
                        SpotLight->SetVisibility(true);
                    }
                }, 0.1f, false);
        }
    }
}

void UFlashlightComponent::PlaySound(USoundBase* Sound)
{
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(this, Sound);
    }
}