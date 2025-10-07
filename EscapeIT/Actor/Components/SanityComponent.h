#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EscapeIT/Data/SanityStructs.h"
#include "SanityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSanityChanged, float, NewSanity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSanityDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSanityLevelChanged, ESanityLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSanityEvent, float, Amount, FString, EventName);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEIT_API USanityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USanityComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PROPERTIES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Base")
    float MaxSanity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Base")
    float MinSanity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity|Base")
    float Sanity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Decay")
    float SanityDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Decay")
    bool bAutoDecay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Recovery")
    float RecoveryDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Recovery")
    float PassiveRecoveryRate;

    UPROPERTY(BlueprintReadWrite, Category = "Sanity|Recovery")
    bool bIsInSafeZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity|State")
    ESanityLevel CurrentSanityLevel;

    // === THRESHOLDS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Thresholds")
    float HighSanityThreshold = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Thresholds")
    float MediumSanityThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Thresholds")
    float LowSanityThreshold = 30.0f;

    // === MULTIPLIERS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Multipliers")
    float DarknessDecayMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Multipliers")
    float RecoveryMultiplier = 1.0f;

    // === EFFECTS ===

    UPROPERTY(BlueprintReadOnly, Category = "Sanity|Effects")
    float VisualEffectIntensity;

    // ==== SAVE GAME ===
    UFUNCTION(BlueprintCallable, Category = "Sanity|Save")
    void LoadFromSaveData(const FSanitySaveData& SaveData);

    UFUNCTION(BlueprintCallable, Category = "Sanity|Save")
    void ResetToCheckpoint(const FSanitySaveData& CheckpointData);

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "Sanity|Events")
    FOnSanityChanged OnSanityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Sanity|Events")
    FOnSanityDepleted OnSanityDepleted;

    UPROPERTY(BlueprintAssignable, Category = "Sanity|Events")
    FOnSanityLevelChanged OnSanityLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Sanity|Events")
    FOnSanityEvent OnSanityEvent;

    UFUNCTION(BlueprintCallable, Category = "Sanity|Save")
    FSanitySaveData CaptureSaveData() const;

    // === GETTERS ===

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sanity")
    float GetSanity() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sanity")
    float GetMinSanity() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sanity")
    float GetMaxSanity() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sanity")
    float GetSanityPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sanity")
    ESanityLevel GetSanityLevel() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sanity")
    float GetVisualEffectIntensity() const;

    // === SETTERS ===

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void SetSanity(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void SetMinSanity(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void SetMaxSanity(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void SetAutoDecay(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void SetIsInSafeZone(bool bSafe);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void SetRecoveryMultiplier(float Multiplier);

    // === CORE FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void ModifySanity(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void RestoreSanity(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void ReduceSanity(float Amount);

    // === EVENT-BASED FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Sanity|Events")
    void ApplySanityEvent(const FSanityEventData& EventData);

    UFUNCTION(BlueprintCallable, Category = "Sanity|Events")
    void OnWitnessHorror(float Amount = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity|Events")
    void OnJumpScare(float Amount = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity|Events")
    void OnPuzzleComplete(float Amount = 15.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity|Events")
    void OnPuzzleFailed(float Amount = 5.0f);

    // === ZONE FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Sanity|Zones")
    void EnterDarkZone();

    UFUNCTION(BlueprintCallable, Category = "Sanity|Zones")
    void ExitDarkZone();

    UFUNCTION(BlueprintCallable, Category = "Sanity|Zones")
    void EnterSafeZone();

    UFUNCTION(BlueprintCallable, Category = "Sanity|Zones")
    void ExitSafeZone();

    // === UTILITY ===

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void ResetSanity();

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    bool IsSanityDepleted() const;

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    bool IsSanityCritical() const;

private:
    void CalculatorSanity(float Amount);
    void UpdateSanity();
    void UpdateSanityLevel();
    void UpdateVisualEffects();

    // Timer cho recovery delay
    FTimerHandle RecoveryTimerHandle;
    void StartRecovery();

    ESanityLevel PreviousSanityLevel;
    bool bIsInDarkZone;
    float CurrentDecayMultiplier;
};