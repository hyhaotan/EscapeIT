// FlashlightComponent.h - IMPROVED VERSION
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightComponent.generated.h"

// Forward declarations
class AFlashlight;
class USpotLightComponent;
class USoundBase;
class UAnimMontage;

// Flashlight state enum for better state management
UENUM(BlueprintType)
enum class EFlashlightState : uint8
{
    Unequipped      UMETA(DisplayName = "Unequipped"),
    Equipping       UMETA(DisplayName = "Equipping"),
    Equipped        UMETA(DisplayName = "Equipped"),
    Unequipping     UMETA(DisplayName = "Unequipping")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashlightStateChanged, EFlashlightState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashlightToggled, bool, bIsOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBatteryChanged, float, CurrentBattery, float, MaxBattery);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBatteryDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBatteryLow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashlightImageChanged, UTexture2D*, NewIcon);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPEIT_API UFlashlightComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFlashlightComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ============================================
    // PUBLIC API - Main Functions
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    bool ToggleLight();

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    bool SetLightEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    bool EquipFlashlight(AFlashlight* FlashlightActor);

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void UnequipFlashlight();

    UFUNCTION(BlueprintCallable, Category = "Flashlight|Battery")
    void AddBatteryCharge(float ChargePercent);

    UFUNCTION(BlueprintCallable, Category = "Flashlight|Battery")
    void ReplaceBattery();

    // ============================================
    // PUBLIC API - Getters
    // ============================================

    UFUNCTION(BlueprintPure, Category = "Flashlight")
    EFlashlightState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Flashlight")
    bool IsEquipped() const { return CurrentState == EFlashlightState::Equipped; }

    UFUNCTION(BlueprintPure, Category = "Flashlight")
    bool IsLightOn() const { return bIsLightOn; }

    UFUNCTION(BlueprintPure, Category = "Flashlight")
    bool CanToggleLight() const;

    UFUNCTION(BlueprintPure, Category = "Flashlight|Battery")
    float GetBatteryPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Flashlight|Battery")
    float GetCurrentBattery() const { return CurrentBattery; }

    UFUNCTION(BlueprintPure, Category = "Flashlight|Battery")
    float GetMaxBatteryDuration() const { return MaxBatteryDuration; }

    UFUNCTION(BlueprintPure, Category = "Flashlight|Battery")
    bool IsBatteryLow() const;

    UFUNCTION(BlueprintPure, Category = "Flashlight|Battery")
    bool IsBatteryDepleted() const;

    UFUNCTION(BlueprintPure, Category = "Flashlight")
    UAnimMontage* GetEquipAnimation() const { return EquipFlashlightAnim; }

    UFUNCTION(BlueprintPure, Category = "Flashlight")
    UAnimMontage* GetUnequipAnimation() const { return UnequipFlashlightAnim; }

    UFUNCTION(BlueprintPure, Category = "Flashlight")
    UTexture2D* GetFlashlightIcon() const;

    // ============================================
    // EVENTS
    // ============================================

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnFlashlightStateChanged OnFlashlightStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnFlashlightToggled OnFlashlightToggled;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnBatteryChanged OnBatteryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnBatteryDepleted OnBatteryDepleted;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnBatteryLow OnBatteryLow;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnFlashlightImageChanged OnFlashlightImageChanged;

    // ============================================
    // CONFIGURATION - Battery
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Battery")
    float MaxBatteryDuration = 300.0f; // 5 minutes

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Battery")
    float DrainRate = 1.0f; // Seconds per second

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Battery", meta = (ClampMin = "0", ClampMax = "100"))
    float LowBatteryThreshold = 20.0f; // Percentage

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Battery")
    float CriticalBatteryThreshold = 5.0f; // Percentage

    // ============================================
    // CONFIGURATION - Light Properties
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Light")
    float NormalIntensity = 10000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Light")
    float LowBatteryIntensity = 3000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Light")
    float LightFadeSpeed = 5.0f; // Speed of fade in/out

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Light")
    bool bRememberLightState = true; // Remember on/off state when re-equipping

    // ============================================
    // CONFIGURATION - Visual Effects
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Effects")
    float FlickerIntensity = 0.3f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Effects")
    float FlickerSpeed = 8.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Effects")
    bool bEnableFlickerEffect = true;

    // ============================================
    // CONFIGURATION - Audio
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Audio")
    USoundBase* ToggleOnSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Audio")
    USoundBase* ToggleOffSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Audio")
    USoundBase* LowBatterySound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Audio")
    USoundBase* LowBatteryBeepSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Audio")
    USoundBase* NoBatterySound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Audio")
    USoundBase* BatteryReplaceSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Audio")
    float LowBatteryBeepIntervalBase = 2.0f;

    // ============================================
    // CONFIGURATION - Animation
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Animation")
    UAnimMontage* EquipFlashlightAnim;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flashlight|Animation")
    UAnimMontage* UnequipFlashlightAnim;

private:
    // ============================================
    // PRIVATE - State Management
    // ============================================

    void SetState(EFlashlightState NewState);
    bool ValidateStateTransition(EFlashlightState NewState) const;
    void OnEquipAnimationComplete();
    void OnUnequipAnimationComplete();

    // ============================================
    // PRIVATE - Light Control
    // ============================================

    void TurnLightOn();
    void TurnLightOff();
    void UpdateLightIntensity(float DeltaTime);
    void ApplyFlickerEffect(float DeltaTime);
    void HandleCriticalBattery();

    // ============================================
    // PRIVATE - Battery Management
    // ============================================

    void UpdateBattery(float DeltaTime);
    void HandleBatteryDepleted();
    void HandleBatteryLow();
    float CalculateTargetIntensity() const;

    // ============================================
    // PRIVATE - Audio
    // ============================================

    void PlaySound(USoundBase* Sound) const;
    void PlayToggleSound();
    void StartLowBatteryBeep();
    void StopLowBatteryBeep();
    void LowBatteryBeep();

    // ============================================
    // PRIVATE - Cleanup
    // ============================================

    void CleanupFlashlight();

    // ============================================
    // PRIVATE - Members
    // ============================================

    UPROPERTY()
    EFlashlightState CurrentState = EFlashlightState::Unequipped;

    UPROPERTY()
    AFlashlight* CurrentFlashlightActor = nullptr;

    UPROPERTY()
    USpotLightComponent* SpotLight = nullptr;

    // State flags
    bool bIsLightOn = false;
    bool bWasLightOnBeforeUnequip = false;
    bool bLowBatterySoundPlayed = false;
    bool bIsFadingLight = false;

    // Battery tracking
    float CurrentBattery = 0.0f;
    float LastBatteryPercentage = 100.0f;

    // Visual effects
    float FlickerTimer = 0.0f;
    float CurrentLightIntensity = 0.0f;
    float TargetLightIntensity = 0.0f;

    // Timers
    FTimerHandle LowBatteryBeepTimer;
    FTimerHandle EquipAnimationTimer;
    FTimerHandle UnequipAnimationTimer;
};