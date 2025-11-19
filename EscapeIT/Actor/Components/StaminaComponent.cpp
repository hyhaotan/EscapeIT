// StaminaComponent.cpp
#include "StaminaComponent.h"
#include "GameFramework/Character.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;

	CurrentStamina = StartingStamina;
	CurrentState = EStaminaState::Normal;
	bIsDraining = false;
	bWasExhausted = false;
	LastDrainTime = 0.0f;
	RegenDelayTimer = 0.0f;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentStamina = FMath::Clamp(StartingStamina, 0.0f, MaxStamina);
	UpdateState();
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateStamina(DeltaTime);
	UpdateState();
}

// ==================== PUBLIC FUNCTIONS ====================

void UStaminaComponent::StartDraining()
{
	if (!bIsDraining)
	{
		bIsDraining = true;
		RegenDelayTimer = 0.0f;
		LastDrainTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	}
}

void UStaminaComponent::StopDraining()
{
	if (bIsDraining)
	{
		bIsDraining = false;
		RegenDelayTimer = 0.0f; // Start delay timer
	}
}

void UStaminaComponent::DrainStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);

	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(CurrentStamina);
	}

	// Reset regen delay when manually draining
	RegenDelayTimer = 0.0f;
	LastDrainTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UStaminaComponent::RestoreStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);

	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(CurrentStamina);
	}
}

bool UStaminaComponent::CanSprint() const
{
	// If exhausted, need to recover more before sprinting again
	if (bWasExhausted)
	{
		return CurrentStamina >= ExhaustedRecoveryThreshold;
	}

	// Normal case: just need minimum stamina
	return CurrentStamina >= MinStaminaToStartSprint;
}

bool UStaminaComponent::IsExhausted() const
{
	return CurrentState == EStaminaState::Exhausted;
}

bool UStaminaComponent::IsDraining() const
{
	return bIsDraining;
}

float UStaminaComponent::GetBreathingIntensity() const
{
	if (!bEnableBreathingEffects)
	{
		return 1.0f;
	}

	// Calculate breathing intensity based on stamina
	float StaminaPercent = GetStaminaPercent() * 100.0f;

	if (StaminaPercent >= LowStaminaThreshold)
	{
		return 1.0f; // Normal breathing
	}

	// Linear interpolation from normal to max intensity
	float Intensity = FMath::Lerp(
		MaxBreathingIntensity,
		1.0f,
		StaminaPercent / LowStaminaThreshold
	);

	return Intensity;
}

// ==================== PRIVATE FUNCTIONS ====================

void UStaminaComponent::UpdateStamina(float DeltaTime)
{
	float OldStamina = CurrentStamina;

	// Drain stamina
	if (bIsDraining && CurrentStamina > 0.0f)
	{
		float DrainAmount = SprintDrainRate * DeltaTime;
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - DrainAmount);
		LastDrainTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	}
	// Regenerate stamina
	else if (ShouldRegenerateStamina())
	{
		float RegenRate = GetCurrentRegenRate();
		float RegenAmount = RegenRate * DeltaTime;
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + RegenAmount);
	}
	else
	{
		// Update regen delay timer
		RegenDelayTimer += DeltaTime;
	}

	// Broadcast change if stamina changed
	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina, 0.1f))
	{
		OnStaminaChanged.Broadcast(CurrentStamina);
	}
}

void UStaminaComponent::UpdateState()
{
	EStaminaState NewState = CurrentState;

	// Determine new state
	if (bIsDraining)
	{
		NewState = EStaminaState::Draining;
	}
	else if (CurrentStamina <= ExhaustionThreshold)
	{
		NewState = EStaminaState::Exhausted;
	}
	else if (CurrentStamina < MaxStamina && !bIsDraining)
	{
		NewState = EStaminaState::Recovering;
	}
	else
	{
		NewState = EStaminaState::Normal;
	}

	// Handle state transition
	if (NewState != CurrentState)
	{
		// Exiting exhausted state
		if (CurrentState == EStaminaState::Exhausted && NewState != EStaminaState::Exhausted)
		{
			// Check if recovered enough
			if (CurrentStamina >= ExhaustedRecoveryThreshold)
			{
				bWasExhausted = false;
				OnStaminaRecovered.Broadcast();
			}
		}

		// Entering exhausted state
		if (NewState == EStaminaState::Exhausted && CurrentState != EStaminaState::Exhausted)
		{
			bWasExhausted = true;
			OnStaminaExhausted.Broadcast();
		}

		ChangeState(NewState);
	}
}

void UStaminaComponent::ChangeState(EStaminaState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}

	CurrentState = NewState;
	OnStaminaStateChanged.Broadcast(CurrentState);

	// Debug log
	FString StateName;
	switch (CurrentState)
	{
	case EStaminaState::Normal:
		StateName = TEXT("Normal");
		break;
	case EStaminaState::Draining:
		StateName = TEXT("Draining");
		break;
	case EStaminaState::Exhausted:
		StateName = TEXT("Exhausted");
		break;
	case EStaminaState::Recovering:
		StateName = TEXT("Recovering");
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("Stamina State Changed: %s (%.1f / %.1f)"), *StateName, CurrentStamina, MaxStamina);
}

float UStaminaComponent::GetCurrentRegenRate() const
{
	// Get owner character to check movement
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return IdleRegenRate;
	}

	// Check if moving
	float Speed = OwnerCharacter->GetVelocity().Size2D();
	bool bIsMoving = Speed > 50.0f;

	// Slower regen when exhausted
	float RegenRate = bIsMoving ? WalkingRegenRate : IdleRegenRate;

	if (CurrentState == EStaminaState::Exhausted || bWasExhausted)
	{
		RegenRate *= ExhaustedRegenMultiplier;
	}

	return RegenRate;
}

bool UStaminaComponent::ShouldRegenerateStamina() const
{
	// Don't regen while draining
	if (bIsDraining)
	{
		return false;
	}

	// Already at max
	if (CurrentStamina >= MaxStamina)
	{
		return false;
	}

	// Wait for regen delay
	if (RegenDelayTimer < RegenDelay)
	{
		return false;
	}

	return true;
}