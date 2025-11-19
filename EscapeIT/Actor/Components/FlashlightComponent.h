
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SpotLightComponent.h"
#include "FlashlightComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBatteryChanged, float, BatteryPercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBatteryDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashlightToggled, bool, bIsOn);

class UAnimMontage;

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
    // PROPERTIES
    // ============================================

    // Light component reference
    UPROPERTY(BlueprintReadWrite, Category = "Flashlight")
    TObjectPtr<USpotLightComponent> SpotLight;

    // Battery system
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Battery")
    float MaxBatteryDuration = 120.0f; // 120 giây (2 phút)

    UPROPERTY(BlueprintReadOnly, Category = "Flashlight|Battery")
    float CurrentBattery = 120.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Battery")
    float DrainRate = 1.0f; // Giảm 1 giây mỗi giây khi bật

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Battery")
    float LowBatteryThreshold = 10.0f; // % để bắt đầu flicker

    // Light properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Light")
    float NormalIntensity = 5000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Light")
    float LowBatteryIntensity = 2000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Light")
    float FlickerSpeed = 5.0f;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "Flashlight")
    bool bIsEquipped = false;

    UPROPERTY(BlueprintReadOnly, Category = "Flashlight")
    bool bIsLightOn = false;

    // Audio
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> ToggleOnSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> ToggleOffSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> BatteryReplaceSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flashlight|Audio")
    TObjectPtr<USoundBase> LowBatterySound;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Flashlight")
    FOnBatteryChanged OnBatteryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight")
    FOnBatteryDepleted OnBatteryDepleted;

    UPROPERTY(BlueprintAssignable, Category = "Flashlight")
    FOnFlashlightToggled OnFlashlightToggled;

    // ============================================
    // PUBLIC FUNCTIONS
    // ============================================

    // Toggle đèn
    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void ToggleLight();

    // Bật/tắt trực tiếp
    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void SetLightEnabled(bool bEnabled);

    // Equip/Unequip
    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void EquipFlashlight();

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void UnequipFlashlight();

    // Battery management
    UFUNCTION(BlueprintCallable, Category = "Flashlight|Battery")
    void ReplaceBattery();

    UFUNCTION(BlueprintCallable, Category = "Flashlight|Battery")
    void AddBatteryCharge(float Amount);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight|Battery")
    float GetBatteryPercentage() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight|Battery")
    bool IsBatteryLow() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight|Battery")
    bool IsBatteryDepleted() const;

    // Query
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight")
    bool IsLightOn() const { return bIsLightOn; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flashlight")
    bool IsEquipped() const { return bIsEquipped; }

protected:
    // ============================================
    // INTERNAL FUNCTIONS
    // ============================================

    void UpdateBattery(float DeltaTime);
    void UpdateLightIntensity();
    void ApplyFlickerEffect(float DeltaTime);
    void PlaySound(USoundBase* Sound);

private:
    float FlickerTimer = 0.0f;
    bool bLowBatterySoundPlayed = false;
    
    UPROPERTY(EditAnywhere,Category=Animation)
    TObjectPtr<UAnimMontage> EquipFlashlightAnim;
    
    UPROPERTY(EditAnywhere,Category=Animation)
    TObjectPtr<UAnimMontage> UnEquipFlashlightAnim;
};