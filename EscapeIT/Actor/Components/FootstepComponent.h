// FootstepComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootstepComponent.generated.h"

class USoundBase;
class ACharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPEIT_API UFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFootstepComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Manually trigger footstep sound (useful for animation notifies) */
	UFUNCTION(BlueprintCallable, Category = "Footstep")
	void PlayFootstepSound();

protected:
	virtual void BeginPlay() override;

private:
	// ==================== REFERENCES ====================
	UPROPERTY()
	ACharacter* OwnerCharacter;

	// ==================== FOOTSTEP SETTINGS ====================
	
	UPROPERTY(EditAnywhere, Category = "Footstep|General")
	bool bEnableFootsteps = true;

	UPROPERTY(EditAnywhere, Category = "Footstep|General")
	bool bAutoPlayFootsteps = true; // Auto play based on movement

	UPROPERTY(EditAnywhere, Category = "Footstep|General")
	float FootstepVolume = 0.5f;

	// Timing
	UPROPERTY(EditAnywhere, Category = "Footstep|Timing")
	float WalkFootstepInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Footstep|Timing")
	float SprintFootstepInterval = 0.3f;
	
	UPROPERTY(EditAnywhere, Category = "Footstep|Timing")
	float MinimumSpeedThreshold = 50.0f; // Minimum speed to play footsteps

	// Line trace settings
	UPROPERTY(EditAnywhere, Category = "Footstep|Detection")
	float TraceDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Footstep|Detection")
	bool bDebugTrace = false;

	// ==================== FOOTSTEP SOUNDS ====================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sounds", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepDefaultSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sounds", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepWoodSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sounds", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepMetalSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sounds", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepConcreteSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sounds", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepGrassSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sounds", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepWaterSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sounds", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepCarpetSFX;

	// ==================== VOLUME MODIFIERS ====================
	
	UPROPERTY(EditAnywhere, Category = "Footstep|Volume")
	float WalkVolumeMultiplier = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Footstep|Volume")
	float SprintVolumeMultiplier = 1.0f;
	

	// ==================== INTERNAL STATE ====================
	
	float LastFootstepTime = 0.0f;
	float CurrentFootstepInterval = 0.5f;

	// ==================== FUNCTIONS ====================
	
	/** Update footstep interval based on movement state */
	void UpdateFootstepInterval();

	/** Get appropriate sound for surface */
	USoundBase* GetFootstepSoundForSurface(const FHitResult& HitResult) const;

	/** Perform line trace to detect surface */
	bool TraceSurface(FHitResult& OutHitResult) const;

	/** Calculate volume based on movement state */
	float GetCurrentVolumeMultiplier() const;

	/** Check if character should play footsteps */
	bool ShouldPlayFootstep() const;
};