// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WindowJumpscareActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UTimelineComponent;
class UCurveFloat;
class USceneComponent;
class USanityComponent;
class UCameraComponent;
class AEscapeITCharacter;
class ACameraActor;
class USpringArmComponent;
class AWidgetManager;
class UPointLightComponent;
class UParticleSystem;
class UAudioComponent;

UCLASS()
class ESCAPEIT_API AWindowJumpscareActor : public AActor
{
	GENERATED_BODY()

public:
	AWindowJumpscareActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	// ========== COMPONENTS ==========
	UPROPERTY(EditAnywhere, Category = "Window | Frame")
	TObjectPtr<UStaticMeshComponent> WindowFrameMesh;

	UPROPERTY(EditAnywhere, Category = "Window | Left")
	TObjectPtr<USceneComponent> LeftWindowHinge;

	UPROPERTY(EditAnywhere, Category = "Window | Left")
	TObjectPtr<UStaticMeshComponent> LeftWindowMesh;

	UPROPERTY(EditAnywhere, Category = "Window | Right")
	TObjectPtr<USceneComponent> RightWindowHinge;

	UPROPERTY(EditAnywhere, Category = "Window | Right")
	TObjectPtr<UStaticMeshComponent> RightWindowMesh;

	UPROPERTY(EditAnywhere, Category = "Actor")
	TObjectPtr<UStaticMeshComponent> GhostMesh;

	UPROPERTY(EditAnywhere, Category = "Box Collission")
	TObjectPtr<UBoxComponent> BoxCollission;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	TObjectPtr<UTimelineComponent> WindowTimeline;

	UPROPERTY(EditAnywhere, Category = "Curve")
	TObjectPtr<UCurveFloat> WindowCurve;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> GhostCamera;

	UPROPERTY()
	TObjectPtr<AActor> OriginalViewTarget;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> GhostSpringArm;

	// ========== NEW: LIGHTING ==========
	UPROPERTY(EditAnywhere, Category = "Visual Effects | Lighting")
	TObjectPtr<UPointLightComponent> FlickerLight;

	// ========== WINDOW SETTINGS ==========
	UPROPERTY(EditAnywhere, Category = "Window | Settings")
	float LeftWindowOpenAngle = -90.0f;

	UPROPERTY(EditAnywhere, Category = "Window | Settings")
	float RightWindowOpenAngle = -90.0f;

	UPROPERTY(EditAnywhere, Category = "Window | Settings")
	float CloseDelayTime = 3.0f;

	// ========== AUDIO ==========
	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> WindowOpenSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TArray<TObjectPtr<USoundBase>> JumpscareSounds;

	UPROPERTY(EditAnywhere, Category = "Audio | Heartbeat")
	TObjectPtr<USoundBase> HeartbeatSound;

	UPROPERTY(EditAnywhere, Category = "Audio | Heartbeat")
	float HeartbeatTriggerDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Audio | Heartbeat")
	float HeartbeatMaxVolume = 1.0f;

	UPROPERTY()
	TObjectPtr<UAudioComponent> HeartbeatAudioComponent;

	// ========== NEW: CAMERA EFFECTS ==========
	UPROPERTY(EditAnywhere, Category = "Camera Effects")
	TSubclassOf<UCameraShakeBase> JumpscareCameraShake;

	UPROPERTY(EditAnywhere, Category = "Camera Effects")
	float CameraShakeScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Camera Effects")
	bool bUseSlowMotion = true;

	UPROPERTY(EditAnywhere, Category = "Camera Effects", meta = (EditCondition = "bUseSlowMotion"))
	float SlowMotionScale = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Camera Effects", meta = (EditCondition = "bUseSlowMotion"))
	float SlowMotionDuration = 0.5f;

	// ========== NEW: PARTICLE EFFECTS ==========
	UPROPERTY(EditAnywhere, Category = "Visual Effects | Particles")
	TObjectPtr<UParticleSystem> GhostAppearEffect;

	UPROPERTY(EditAnywhere, Category = "Visual Effects | Particles")
	TObjectPtr<UParticleSystem> WindowBurstEffect;

	// ========== NEW: LIGHTING EFFECTS ==========
	UPROPERTY(EditAnywhere, Category = "Visual Effects | Lighting")
	bool bUseFlickerEffect = true;

	UPROPERTY(EditAnywhere, Category = "Visual Effects | Lighting", meta = (EditCondition = "bUseFlickerEffect"))
	float FlickerIntensity = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Visual Effects | Lighting", meta = (EditCondition = "bUseFlickerEffect"))
	FLinearColor FlickerColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, Category = "Visual Effects | Lighting", meta = (EditCondition = "bUseFlickerEffect"))
	float FlickerDuration = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Visual Effects | Lighting", meta = (EditCondition = "bUseFlickerEffect"))
	int32 FlickerCount = 3;

	// ========== NEW: GHOST VARIATIONS ==========
	UPROPERTY(EditAnywhere, Category = "Actor | Variations")
	TArray<TObjectPtr<UStaticMesh>> GhostMeshVariations;

	UPROPERTY(EditAnywhere, Category = "Actor | Variations")
	bool bUseRandomGhost = false;

	// ========== NEW: MULTI-STAGE ==========
	UPROPERTY(EditAnywhere, Category = "Window | Multi-Stage")
	bool bUseMultiStage = false;

	UPROPERTY(EditAnywhere, Category = "Window | Multi-Stage", meta = (EditCondition = "bUseMultiStage"))
	float Stage1Delay = 0.5f; // Delay before window starts opening

	UPROPERTY(EditAnywhere, Category = "Window | Multi-Stage", meta = (EditCondition = "bUseMultiStage"))
	float Stage2Delay = 1.0f; // Delay before ghost appears

	UPROPERTY(EditAnywhere, Category = "Window | Multi-Stage", meta = (EditCondition = "bUseMultiStage"))
	float Stage1WindowOpenPercent = 0.3f; // How much window opens in stage 1 (0-1)

	// ========== NEW: INTENSITY SCALING ==========
	UPROPERTY(EditAnywhere, Category = "Window | Settings")
	bool bUseDistanceScaling = true;

	UPROPERTY(EditAnywhere, Category = "Window | Settings", meta = (EditCondition = "bUseDistanceScaling"))
	float MaxIntensityDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Window | Settings", meta = (EditCondition = "bUseDistanceScaling"))
	float MinIntensityDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Window | Settings")
	float BaseSanityReduction = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Window | Settings", meta = (EditCondition = "bUseDistanceScaling"))
	float MaxSanityReduction = 40.0f;

	// ========== INTERNAL STATE ==========
	FRotator LeftHingeInitialRot;
	FRotator RightHingeInitialRot;
	FTimerHandle CloseWindowTimer;
	FTimerHandle SlowMotionTimer;
	FTimerHandle FlickerTimer;
	FTimerHandle Stage1Timer;
	FTimerHandle Stage2Timer;
	bool bHasTriggered = false;
	bool bIsPlayerNearby = false;
	float CurrentJumpscareIntensity = 1.0f;

	UPROPERTY()
	TObjectPtr<USanityComponent> SanityComponent;

	UPROPERTY()
	TObjectPtr<AWidgetManager> WidgetManager;

	UPROPERTY()
	TObjectPtr<AEscapeITCharacter> OwningCharacter;

	// ========== FUNCTIONS ==========
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void UpdateWindowRotation(float Value);

	void TriggerJumpscare();
	void CloseWindows();

	// ========== NEW FUNCTIONS ==========
	void UpdateHeartbeat(float DeltaTime);
	float CalculateJumpscareIntensity(float Distance);
	void ApplyCameraShake();
	void ApplySlowMotion();
	void RestoreTimeScale();
	void SpawnParticleEffects();
	void TriggerFlickerEffect();
	void FlickerLightStep(int32 CurrentFlicker);
	void SelectRandomGhost();
	USoundBase* GetRandomJumpscareSound();

	// Multi-stage functions
	void ExecuteStage1();
	void ExecuteStage2();
	void ExecuteFinalStage();
};