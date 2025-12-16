// StaminaComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

UENUM(BlueprintType)
enum class EStaminaState : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Draining UMETA(DisplayName = "Draining"),
	Exhausted UMETA(DisplayName = "Exhausted"),
	Recovering UMETA(DisplayName = "Recovering")
};

// Event delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaStateChanged, EStaminaState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaExhausted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaRecovered);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPEIT_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== PUBLIC FUNCTIONS ====================
	
	/** Start draining stamina */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StartDraining();

	/** Stop draining stamina */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StopDraining();

	/** Instantly drain stamina by amount */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void DrainStamina(float Amount);

	/** Instantly restore stamina by amount */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void RestoreStamina(float Amount);

	/** Check if character can sprint */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool CanSprint() const;

	/** Check if stamina is exhausted */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool IsExhausted() const;

	/** Check if stamina is draining */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool IsDraining() const;

	// ==================== GETTERS ====================
	
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetStaminaPercent() const { return CurrentStamina / MaxStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	EStaminaState GetStaminaState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetBreathingIntensity() const;

protected:
	virtual void BeginPlay() override;

private:
	// ==================== STAMINA SETTINGS ====================
	
	UPROPERTY(EditAnywhere, Category = "Stamina|General")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina|General")
	float StartingStamina = 100.0f;

	// Drain rates
	UPROPERTY(EditAnywhere, Category = "Stamina|Drain")
	float SprintDrainRate = 20.0f; // Stamina per second when sprinting

	UPROPERTY(EditAnywhere, Category = "Stamina|Drain")
	float JumpStaminaCost = 10.0f; // Instant cost when jumping

	// Regeneration
	UPROPERTY(EditAnywhere, Category = "Stamina|Regeneration")
	float IdleRegenRate = 15.0f; // Stamina per second when idle

	UPROPERTY(EditAnywhere, Category = "Stamina|Regeneration")
	float WalkingRegenRate = 8.0f; // Stamina per second when walking

	UPROPERTY(EditAnywhere, Category = "Stamina|Regeneration")
	float RegenDelay = 1.5f; // Delay before regen starts after draining stops

	// Exhaustion
	UPROPERTY(EditAnywhere, Category = "Stamina|Exhaustion")
	float ExhaustionThreshold = 5.0f; // Can't sprint below this

	UPROPERTY(EditAnywhere, Category = "Stamina|Exhaustion")
	float ExhaustedRecoveryThreshold = 30.0f; // Must reach this to sprint again after exhaustion

	UPROPERTY(EditAnywhere, Category = "Stamina|Exhaustion")
	float ExhaustedRegenMultiplier = 0.5f; // Slower regen when exhausted

	// Sprint requirements
	UPROPERTY(EditAnywhere, Category = "Stamina|Sprint")
	float MinStaminaToStartSprint = 20.0f; // Minimum stamina to start sprinting

	UPROPERTY(EditAnywhere, Category = "Stamina|Sprint")
	float MinStaminaToContinueSprint = 5.0f; // Can continue sprinting until this threshold

	// ==================== BREATHING EFFECTS ====================
	
	UPROPERTY(EditAnywhere, Category = "Stamina|Effects")
	bool bEnableBreathingEffects = true;

	UPROPERTY(EditAnywhere, Category = "Stamina|Effects")
	float LowStaminaThreshold = 30.0f; // Below this, breathing becomes heavy

	UPROPERTY(EditAnywhere, Category = "Stamina|Effects")
	float MaxBreathingIntensity = 2.0f; // Multiplier for breathing when exhausted

	// ==================== INTERNAL STATE ====================
	
	UPROPERTY(VisibleAnywhere, Category = "Stamina|Debug")
	float CurrentStamina;

	UPROPERTY(VisibleAnywhere, Category = "Stamina|Debug")
	EStaminaState CurrentState;

	bool bIsDraining = false;
	bool bWasExhausted = false;
	float LastDrainTime = 0.0f;
	float RegenDelayTimer = 0.0f;

	// ==================== EVENTS ====================
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaStateChanged OnStaminaStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaExhausted OnStaminaExhausted;

	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaRecovered OnStaminaRecovered;

private:
	// ==================== FUNCTIONS ====================
	
	void UpdateStamina(float DeltaTime);
	void UpdateState();
	void ChangeState(EStaminaState NewState);
	float GetCurrentRegenRate() const;
	bool ShouldRegenerateStamina() const;
};