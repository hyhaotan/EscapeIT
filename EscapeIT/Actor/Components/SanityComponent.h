#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "EscapeIT/Data/SanityStructs.h"
#include "SanityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSanityChanged, float, NewSanity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSanityDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSanityEvent, float, Amount, const FString&, EventName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSanityLevelChanged, ESanityLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPanicPostProcessUpdated, float, VignetteAmount, float, MotionBlurAmount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEIT_API USanityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USanityComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Events ---
    UPROPERTY(BlueprintAssignable)
    FOnSanityChanged OnSanityChanged;

    UPROPERTY(BlueprintAssignable)
    FOnSanityDepleted OnSanityDepleted;

    UPROPERTY(BlueprintAssignable)
    FOnSanityEvent OnSanityEvent;

    UPROPERTY(BlueprintAssignable)
    FOnSanityLevelChanged OnSanityLevelChanged;

    // Broadcast when panic post-process values change (bind in BP to update materials/postprocess volumes)
    UPROPERTY(BlueprintAssignable)
    FOnPanicPostProcessUpdated OnPanicPostProcessUpdated;

    // --- Getters ---
    UFUNCTION(BlueprintCallable)
    float GetSanity() const;

    UFUNCTION(BlueprintCallable)
    float GetMinSanity() const;

    UFUNCTION(BlueprintCallable)
    float GetMaxSanity() const;

    UFUNCTION(BlueprintCallable)
    float GetSanityPercent() const; // 0..1

    UFUNCTION(BlueprintCallable)
    ESanityLevel GetSanityLevel() const;

    UFUNCTION(BlueprintCallable)
    float GetVisualEffectIntensity() const;

    // --- Setters ---
    UFUNCTION(BlueprintCallable)
    void SetSanity(float Amount);

    UFUNCTION(BlueprintCallable)
    void SetMinSanity(float Amount);

    UFUNCTION(BlueprintCallable)
    void SetMaxSanity(float Amount);

    UFUNCTION(BlueprintCallable)
    void SetAutoDecay(bool bEnabled);

    UFUNCTION(BlueprintCallable)
    void SetIsInSafeZone(bool bSafe);

    UFUNCTION(BlueprintCallable)
    void SetRecoveryMultiplier(float Multiplier);

    // Core
    UFUNCTION(BlueprintCallable)
    void ModifySanity(float Amount);

    UFUNCTION(BlueprintCallable)
    void RestoreSanity(float Amount);

    UFUNCTION(BlueprintCallable)
    void ReduceSanity(float Amount);

    UFUNCTION(BlueprintCallable)
    void ApplySanityEvent(const FSanityEventData& EventData);

    // Zone helpers
    UFUNCTION(BlueprintCallable)
    void EnterDarkZone();

    UFUNCTION(BlueprintCallable)
    void ExitDarkZone();

    UFUNCTION(BlueprintCallable)
    void EnterSafeZone();

    UFUNCTION(BlueprintCallable)
    void ExitSafeZone();

    UFUNCTION(BlueprintCallable)
    void ResetSanity();

    UFUNCTION(BlueprintCallable)
    bool IsSanityDepleted() const;

    UFUNCTION(BlueprintCallable)
    bool IsSanityCritical() const;

    // === EVENT-BASED FUNCTIONS ===
    UFUNCTION()
    void OnWitnessHorror(float Amount);

    UFUNCTION()
    void OnJumpScare(float Amount);

    UFUNCTION()
    void OnPuzzleComplete(float Amount);

    UFUNCTION()
    void OnPuzzleFailed(float Amount);

    // Save/Load
    FSanitySaveData CaptureSaveData() const;
    void LoadFromSaveData(const FSanitySaveData& SaveData);
    void ResetToCheckpoint(const FSanitySaveData& CheckpointData);

protected:
    // Core values
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
    float MaxSanity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
    float MinSanity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity")
    float Sanity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
    float SanityDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
    bool bAutoDecay;

    // Recovery
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Recovery")
    float RecoveryDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Recovery")
    float PassiveRecoveryRate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity|Zones")
    bool bIsInSafeZone;

    // internal flag set after RecoveryDelay
    bool bIsRecovering;

    // Sanity levels
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity|Levels")
    ESanityLevel CurrentSanityLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity|Levels")
    ESanityLevel PreviousSanityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Levels")
    float HighSanityThreshold; // percent 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Levels")
    float MediumSanityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Levels")
    float LowSanityThreshold;

    // Dark zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Zones")
    float DarknessDecayMultiplier;

    bool bIsInDarkZone;
    float CurrentDecayMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Recovery")
    float RecoveryMultiplier;

    // Visual effect
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity|Visual")
    float VisualEffectIntensity;

    // Camera effects
    UPROPERTY()
    class AEscapeITCharacter* OwnerCharacter;

    UPROPERTY()
    UCameraComponent* CameraComponent;

    UPROPERTY()
    APlayerCameraManager* PlayerCameraManager;

    // Camera shake class for heartbeat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Camera")
    TSubclassOf<UCameraShakeBase> HeartbeatCameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Camera")
    float HeartbeatShakeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Camera")
    float DizzyRotationAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Camera")
    float DizzyPulseSpeed;

    bool bIsCameraEffectActive;
    float CameraEffectElapsedTime;

    FTimerHandle RecoveryTimerHandle;

    // --- Internal utilities ---
    void CalculatorSanity(float Amount);
    void UpdateSanity();
    void UpdateSanityLevel();
    void UpdateVisualEffects();
    void StartRecovery();

    // Camera-related helpers
    void UpdateCameraEffects();
    void StartCameraPanicEffect();
    void StopCameraPanicEffect();
    void ApplyCameraHeartbeatShake(float DeltaTime);
    void ApplyCameraDizzyEffect(float DeltaTime);
    void ApplyPanicPostProcess(float SanityPercent);
    void ApplyVignetteEffect(float SanityPercent);
    void ApplyMotionBlur(float SanityPercent);
};
