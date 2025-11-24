#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightComponent.generated.h"

// Forward declarations
class USpotLightComponent;
class USoundBase;
class UAnimMontage;

// ============================================
// DELEGATES
// ============================================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashlightToggled, bool, bIsOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBatteryChanged, float, CurrentBattery, float, MaxBattery);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBatteryDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBatteryLow);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEIT_API UFlashlightComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFlashlightComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ============================================
    // PROPERTIES - LIGHT COMPONENT
    // ============================================
    
    /** Reference to the SpotLight component */
    UPROPERTY(BlueprintReadOnly, Category = "Flashlight")
    TObjectPtr<USpotLightComponent> SpotLight;

    // ============================================
    // PROPERTIES - BATTERY SYSTEM
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Battery")
    float MaxBatteryDuration = 120.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Flashlight|Battery")
    float CurrentBattery = 120.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Battery")
    float DrainRate = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Battery")
    float LowBatteryThreshold = 20.0f;

    // ============================================
    // PROPERTIES - LIGHT SETTINGS
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Light")
    float NormalIntensity = 5000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Light")
    float LowBatteryIntensity = 2000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Light")
    float FlickerSpeed = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Light")
    float FlickerIntensity = 0.3f;

    // ============================================
    // PROPERTIES - STATE
    // ============================================

    UPROPERTY(BlueprintReadOnly, Category = "Flashlight|State")
    bool bIsEquipped = false;

    UPROPERTY(BlueprintReadOnly, Category = "Flashlight|State")
    bool bIsLightOn = false;

    // ============================================
    // PROPERTIES - AUDIO
    // ============================================
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> ToggleOnSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> ToggleOffSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> BatteryReplaceSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> LowBatterySound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> NoBatterySound;

    // ============================================
    // PROPERTIES - ANIMATION
    // ============================================
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Animation")
    TObjectPtr<UAnimMontage> EquipFlashlightAnim;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Animation")
    TObjectPtr<UAnimMontage> UnequipFlashlightAnim;

    // ============================================
    // DELEGATES
    // ============================================

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnFlashlightToggled OnFlashlightToggled;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnBatteryChanged OnBatteryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnBatteryDepleted OnBatteryDepleted;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight|Events")
    FOnBatteryLow OnBatteryLow;

    // ============================================
    // PUBLIC FUNCTIONS - TOGGLE & CONTROL
    // ============================================
    
    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    bool ToggleLight();
    
    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    bool SetLightEnabled(bool bEnabled);
    
    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void EquipFlashlight();

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void UnequipFlashlight();

    // ============================================
    // PUBLIC FUNCTIONS - BATTERY MANAGEMENT
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Flashlight|Battery")
    void ReplaceBattery();

    UFUNCTION(BlueprintCallable, Category = "Flashlight|Battery")
    void AddBatteryCharge(float Amount);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight|Battery")
    float GetBatteryPercentage() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight|Battery")
    float GetBatteryDuration() const { return CurrentBattery; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight|Battery")
    bool IsBatteryLow() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight|Battery")
    bool IsBatteryDepleted() const;

    // ============================================
    // PUBLIC FUNCTIONS - QUERY STATE
    // ============================================

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight")
    bool IsLightOn() const { return bIsLightOn; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight")
    bool IsEquipped() const { return bIsEquipped; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight")
    bool CanToggleLight() const;

protected:
    // ============================================
    // INTERNAL FUNCTIONS - BATTERY
    // ============================================
    
    void UpdateBattery(float DeltaTime);
    void UpdateLightIntensity();

    // ============================================
    // INTERNAL FUNCTIONS - VISUAL EFFECTS
    // ============================================
    
    void ApplyFlickerEffect(float DeltaTime);
    void HandleCriticalBattery();

    // ============================================
    // INTERNAL FUNCTIONS - AUDIO
    // ============================================

    void PlaySound(USoundBase* Sound);
    void PlayToggleSound();

private:
    // ============================================
    // PRIVATE VARIABLES
    // ============================================
    
    float FlickerTimer = 0.0f;
    bool bLowBatterySoundPlayed = false;
    float LastBatteryPercentage = 100.0f;
};