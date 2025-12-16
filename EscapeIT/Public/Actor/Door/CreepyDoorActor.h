// CreepyDoorActor.h
#pragma once

#include "CoreMinimal.h"
#include "Door.h"
#include "CreepyDoorActor.generated.h"

class UPointLightComponent;
class UParticleSystemComponent;
class UParticleSystem;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class ESCAPEIT_API ACreepyDoorActor : public ADoor
{
	GENERATED_BODY()

public:
	ACreepyDoorActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// COMPONENTS
	// ============================================================================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creepy Door|Lighting")
	UPointLightComponent* DoorLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creepy Door|Particles")
	UParticleSystemComponent* DustParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creepy Door|Particles")
	UParticleSystemComponent* FogParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creepy Door|Shadow")
	UStaticMeshComponent* ShadowFigure;

	// ============================================================================
	// ASSETS
	// ============================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Particles")
	UParticleSystem* DustEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Particles")
	UParticleSystem* FogEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Shadow")
	UMaterialInterface* ShadowMaterial;

	// ============================================================================
	// SETTINGS
	// ============================================================================
	
	// --- Shake Settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Shake")
	float ShakeDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Shake")
	float ShakeIntensity = 2.0f;

	// --- Light Settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Lighting")
	float LightIntensityMin = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Lighting")
	float LightIntensityMax = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Lighting")
	float LightFlickerSpeed = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Lighting")
	FLinearColor LightColorStart = FLinearColor(1.0f, 0.1f, 0.0f); // Đỏ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Lighting")
	FLinearColor LightColorEnd = FLinearColor(0.0f, 0.5f, 1.0f); // Xanh lạnh

	// --- Shadow Settings ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Shadow")
	FVector ShadowStartLocation = FVector(-200.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Shadow")
	FVector ShadowEndLocation = FVector(-50.0f, 0.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|Shadow")
	float ShadowMoveDuration = 3.0f;

	// --- General Settings ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|General")
	bool bEnableRandomBehavior = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy Door|General", 
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PauseAtProgress = 0.5f; // Tạm dừng ở 50% animation

	// ============================================================================
	// MAIN FUNCTIONS
	// ============================================================================
	
	UFUNCTION(BlueprintCallable, Category = "Creepy Door")
	void OpenDoorWithCreepyEffects();

	virtual void UpdateDoorRotation_Implementation(float Value) override;
	virtual void CloseDoor_Implementation() override;

protected:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================
	
	FRotator OriginalDoorRotation;
	
	// Shake state
	float CurrentShakeTime;
	bool bIsShaking;

	// Light state
	float LightFlickerTime;

	// Shadow state
	float ShadowMoveProgress;
	UMaterialInstanceDynamic* ShadowDynamicMaterial; // FIX: Lưu material để tránh leak

	// Door state
	bool bHasPaused;

	// Timers
	FTimerHandle ShakeTimerHandle;
	FTimerHandle LightFlickerTimerHandle;
	FTimerHandle PauseTimerHandle;
	FTimerHandle RandomCloseTimerHandle;

	// ============================================================================
	// INTERNAL FUNCTIONS
	// ============================================================================
	
	// Shake
	void StartDoorShake();
	void UpdateDoorShake(float DeltaTime); // FIX: Thêm DeltaTime
	void StopDoorShake();

	// Light
	void StartLightFlicker();
	void UpdateLightFlicker();
	void TransitionLightColor(float Progress);

	// Particles
	void ActivateParticleEffects();

	// Random behavior
	void PauseDoorAtHalf();
	void ResumeDoorOpening();
	void CloseDoorRandomly();

	// Shadow
	void ShowShadowFigure();
	void UpdateShadowMovement(float DeltaTime); // FIX: Thêm DeltaTime
	void HideShadowFigure();

	// Sound
	void PlayCreepySound(USoundBase* Sound);

	// FIX: Helper function
	void ClearAllTimers();
};