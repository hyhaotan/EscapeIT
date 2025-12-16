// HeaderBobComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actor/Components/SanityComponent.h"
#include "HeaderBobComponent.generated.h"

UENUM(BlueprintType)
enum class EHeaderBobType : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Walk UMETA(DisplayName = "Walk"),
	Sprint UMETA(DisplayName = "Sprint")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPEIT_API UHeaderBobComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeaderBobComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Header Bob")
	void TriggerShockBob();

	UFUNCTION(BlueprintCallable, Category = "Header Bob")
	void TriggerLandingImpact(float FallSpeed);

protected:
	virtual void BeginPlay() override;

private:
	// ==================== REFERENCES ====================
	UPROPERTY()
	class AEscapeITCharacter* OwnerCharacter;

	UPROPERTY()
	class UCameraComponent* CameraComponent;

	UPROPERTY()
	class USanityComponent* SanityComponent;

	UPROPERTY()
	class UAudioComponent* HeartbeatAudioComponent;
	
	UPROPERTY()
	class UStaminaComponent* StaminaComponent;

	// ==================== BOB SETTINGS ====================
	UPROPERTY(EditAnywhere, Category = "Header Bob|Idle")
	float IdleAmplitude = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Header Bob|Idle")
	float IdleFrequency = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Header Bob|Walk")
	float WalkAmplitude = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Header Bob|Walk")
	float WalkFrequency = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Header Bob|Sprint")
	float SprintAmplitude = 3.5f;

	UPROPERTY(EditAnywhere, Category = "Header Bob|Sprint")
	float SprintFrequency = 3.5f;

	UPROPERTY(EditAnywhere, Category = "Header Bob|General")
	float TransitionSpeed = 5.0f;

	// ==================== FOV DYNAMICS ====================
	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float DefaultFOV = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float WalkFOV = 92.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float SprintFOV = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float FOVInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	bool bEnableFOVDynamics = true;

	float CurrentFOV = 90.0f;
	float TargetFOV = 90.0f;

	// ==================== CAMERA TILT ====================
	UPROPERTY(EditAnywhere, Category = "Camera|Tilt")
	bool bEnableCameraTilt = true;

	UPROPERTY(EditAnywhere, Category = "Camera|Tilt")
	float TiltAmount = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Camera|Tilt")
	float TiltInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Tilt")
	float MaxTiltAngle = 3.0f;

	float CurrentCameraTilt = 0.0f;
	float TargetCameraTilt = 0.0f;

	// ==================== LANDING IMPACT ====================
	UPROPERTY(EditAnywhere, Category = "Camera|Landing")
	bool bEnableLandingImpact = true;

	UPROPERTY(EditAnywhere, Category = "Camera|Landing")
	float LandingImpactThreshold = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Landing")
	float LandingImpactStrength = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Landing")
	float LandingFOVPunch = -10.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Landing")
	TSubclassOf<class UCameraShakeBase> LandingCameraShakeClass;

	bool bIsLandingImpactActive = false;
	float LandingImpactIntensity = 0.0f;

	// ==================== BREATHING ====================
	UPROPERTY(EditAnywhere, Category = "Camera|Breathing")
	bool bEnableBreathing = true;

	UPROPERTY(EditAnywhere, Category = "Camera|Breathing")
	float BreathingAmplitude = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Camera|Breathing")
	float BreathingFrequency = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Camera|Breathing")
	float StaminaBreathingMultiplier = 2.5f;

	float BreathingTimer = 0.0f;

	// ==================== SANITY EFFECTS ====================
	UPROPERTY(EditAnywhere, Category = "Sanity")
	float SanityMultiplier_Medium = 1.3f;

	UPROPERTY(EditAnywhere, Category = "Sanity")
	float SanityMultiplier_Low = 1.8f;

	UPROPERTY(EditAnywhere, Category = "Sanity")
	float SanityMultiplier_Critical = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Sanity")
	float FrequencyMultiplier_Low = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Sanity")
	float FrequencyMultiplier_Critical = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Sanity|Idle Vibration")
	bool bEnableIdleVibration = true;

	UPROPERTY(EditAnywhere, Category = "Sanity|Idle Vibration")
	float IdleVibrationThreshold = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Sanity|Idle Vibration")
	float IdleVibrationAmplitude = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Sanity|Idle Vibration")
	float IdleVibrationFrequency = 4.0f;

	// ==================== SHOCK EFFECT ====================
	UPROPERTY(EditAnywhere, Category = "Shock")
	float ShockIntensity = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Shock")
	float ShockDuration = 1.5f;

	float CurrentShockIntensity = 0.0f;
	float ShockElapsedTime = 0.0f;
	bool bIsInShock = false;

	// ==================== ENTITY PROXIMITY ====================
	UPROPERTY(EditAnywhere, Category = "Entity Proximity")
	bool bEnableEntityProximity = true;

	UPROPERTY(EditAnywhere, Category = "Entity Proximity")
	float EntityProximityThreshold = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Entity Proximity")
	float EntityProximityMultiplier = 1.8f;

	float DistanceToEntity = 10000.0f;
	bool bIsEntityNear = false;

	// ==================== HEARTBEAT AUDIO ====================
	UPROPERTY(EditAnywhere, Category = "Audio|Heartbeat")
	bool bEnableHeartbeatAudio = true;

	UPROPERTY(EditAnywhere, Category = "Audio|Heartbeat")
	USoundBase* HeartbeatSFX;

	UPROPERTY(EditAnywhere, Category = "Audio|Heartbeat")
	float HeartbeatVolumeMultiplier = 0.5f;

	float HeartbeatTimer = 0.0f;

	// ==================== SCREEN EFFECTS ====================
	UPROPERTY(EditAnywhere, Category = "Screen Effects")
	bool bEnableVignette = true;

	UPROPERTY(EditAnywhere, Category = "Screen Effects")
	bool bEnableChromaticAberration = true;

	UPROPERTY(EditAnywhere, Category = "Screen Effects")
	bool bEnableScreenShake = true;

	UPROPERTY(EditAnywhere, Category = "Screen Effects")
	float ScreenShakeIntensity = 1.0f;

	// ==================== INTERNAL STATE ====================
	FVector OriginalCameraLocation;
	FVector CurrentCameraOffset;
	FVector TargetCameraOffset;
	FRotator OriginalCameraRotation;

	float BobTimer;
	EHeaderBobType CurrentBobType;
	EHeaderBobType TargetBobType;
	float CurrentShakeIntensity;
	float TargetShakeIntensity;

	// ==================== FUNCTIONS ====================
	void UpdateHeaderBobType();
	EHeaderBobType GetCurrentBobType(float CharacterSpeed) const;
	void UpdateCameraShake();
	void ApplyCameraBob(float DeltaTime);

	// FOV & Camera
	void UpdateFOVDynamics(float DeltaTime);
	void UpdateCameraTilt(float DeltaTime);
	void UpdateBreathing(float DeltaTime);

	// Landing
	void UpdateLandingImpact(float DeltaTime);

	// Sanity
	float GetSanityMultiplier(float SanityPercent) const;
	float GetFrequencyMultiplier(float SanityPercent) const;
	float GetCurrentSanityPercent() const;

	// Shock
	void UpdateShockEffect(float DeltaTime);

	// Entity
	void UpdateEntityProximity();
	AActor* FindNearestEntity() const;

	// Audio
	void UpdateHeartbeatAudio(float SanityPercent);
	void PlayHeartbeat(float Volume);
	void StopHeartbeat();

	// Screen Effects
	void ApplyScreenEffects(float SanityPercent);
	void UpdateVignette(float Intensity);
	void UpdateChromaticAberration(float Intensity);
	void ApplyScreenShake(float Intensity);

	// Events
	UFUNCTION()
	void OnSanityLevelChanged(ESanityLevel NewSanityLevel);
	void OnSanityRecovered();
};