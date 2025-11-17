#pragma once

#include "CoreMinimal.h"
#include "Door.h"
#include "CreepyDoorActor.generated.h"

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

	// ============== Âm thanh + Shake ==============
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Sound")
	class USoundBase* CreakSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Sound")
	class USoundBase* SqueezeSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Sound")
	class USoundBase* WhisperSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Shake")
	float ShakeIntensity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Shake")
	float ShakeDuration = 0.5f;

	// ============== Ánh sáng thay đổi ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creepy|Components")
	class UPointLightComponent* DoorLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Light")
	float LightFlickerSpeed = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Light")
	float LightIntensityMin = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Light")
	float LightIntensityMax = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Light")
	FLinearColor LightColorStart = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Light")
	FLinearColor LightColorEnd = FLinearColor(0.0f, 0.5f, 1.0f); // Xanh lạnh

	// ============== Particle Effects ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creepy|Components")
	class UParticleSystemComponent* DustParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creepy|Components")
	class UParticleSystemComponent* FogParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Particles")
	class UParticleSystem* DustEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Particles")
	class UParticleSystem* FogEffect;

	// ============== Cửa tự đóng mở bất thường ==============
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Behavior")
	bool bEnableRandomBehavior = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Behavior")
	float PauseAtProgress = 0.5f; // Dừng ở 50%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Behavior")
	float PauseDuration = 1.0f;

	// ============== Bóng đổ/Silhouette ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creepy|Components")
	class UStaticMeshComponent* ShadowFigure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Shadow")
	class UMaterialInterface* ShadowMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Shadow")
	float ShadowMoveDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Shadow")
	FVector ShadowStartLocation = FVector(-50.0f, 0.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creepy|Shadow")
	FVector ShadowEndLocation = FVector(-50.0f, 100.0f, 100.0f);

	// ============== FUNCTIONS ==============
	UFUNCTION(BlueprintCallable, Category = "Creepy")
	void OpenDoorWithCreepyEffects();

private:
	// Shake
	void StartDoorShake();
	void UpdateDoorShake();
	void StopDoorShake();

	// Light flicker
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
	void UpdateShadowMovement();
	void HideShadowFigure();

	virtual void UpdateDoorRotation_Implementation(float Value) override;
	
	// Play sounds
	void PlayCreepySound(USoundBase* Sound);

	// Timer handles
	FTimerHandle ShakeTimerHandle;
	FTimerHandle LightFlickerTimerHandle;
	FTimerHandle PauseTimerHandle;
	FTimerHandle ShadowMoveTimerHandle;
	FTimerHandle RandomCloseTimerHandle;

	// State variables
	float CurrentShakeTime;
	bool bIsShaking;
	float LightFlickerTime;
	float ShadowMoveProgress;
	bool bHasPaused;
	FRotator OriginalDoorRotation;

	virtual void CloseDoor_Implementation() override;
};