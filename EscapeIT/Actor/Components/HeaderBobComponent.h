// HeaderBobComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EscapeIT/Actor/Components/SanityComponent.h"
#include "HeaderBobComponent.generated.h"

class UCameraComponent;
class AEscapeITCharacter;
class USanityComponent;
class UCameraShakeBase;

UENUM()
enum class EHeaderBobType : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Walk UMETA(DisplayName = "Walk")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEIT_API UHeaderBobComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeaderBobComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== CAMERA SHAKE ASSETS ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> IdleShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> WalkShake;

	// ==================== HEADER BOB SETTINGS ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Header Bob|Walk")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Header Bob|Walk")
	float WalkAmplitude = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Header Bob|Walk")
	float WalkFrequency = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Header Bob|Idle")
	float IdleAmplitude = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Header Bob|Idle")
	float IdleFrequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Header Bob|General")
	float TransitionSpeed = 5.0f;

	// ==================== SANITY MODIFIERS ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity Modifiers|Multipliers")
	float SanityMultiplier_Medium = 1.0f; // 70-50%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity Modifiers|Multipliers")
	float SanityMultiplier_Low = 1.3f; // 50-30%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity Modifiers|Multipliers")
	float SanityMultiplier_Critical = 1.8f; // <30%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity Modifiers|Frequency")
	float FrequencyMultiplier_Low = 1.2f; // <50%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity Modifiers|Frequency")
	float FrequencyMultiplier_Critical = 1.5f; // <30%

	// ==================== IDLE STATE WITH LOW SANITY ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Low Sanity")
	bool bEnableIdleVibration = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Low Sanity")
	float IdleVibrationThreshold = 30.0f; // Kích hoạt khi Sanity < 30%

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Low Sanity")
	float IdleVibrationAmplitude = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Low Sanity")
	float IdleVibrationFrequency = 3.0f;

	// ==================== SHOCK BOB SETTINGS ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shock Effect")
	float ShockIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shock Effect")
	float ShockDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shock Effect")
	float ShockDecayRate = 2.0f;

	// ==================== ENTITY PROXIMITY SETTINGS ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Proximity")
	bool bEnableEntityProximity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Proximity")
	float EntityProximityThreshold = 500.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Proximity")
	float EntityProximityMultiplier = 2.0f;

	// ==================== AUDIO SETTINGS ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bEnableHeartbeatAudio = true;

	// You can keep this as a direct pointer (asset reference) or use TSoftObjectPtr<USoundBase> for deferred loading.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* HeartbeatSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float HeartbeatVolumeMultiplier = 1.0f;

	// ==================== VIGNETTE & SCREEN EFFECTS ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
	bool bEnableVignette = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
	bool bEnableChromaticAberration = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
	bool bEnableScreenShake = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
	float ScreenShakeIntensity = 0.5f;

	// ==================== PUBLIC FUNCTIONS ====================
	// Called when jumpscare appears
	UFUNCTION(BlueprintCallable, Category = "Header Bob")
	void TriggerShockBob();

	// Called when the character recover sanity
	UFUNCTION(BlueprintCallable, Category = "Header Bob")
	void OnSanityRecovered();

	// Take current sanity value
	UFUNCTION(BlueprintCallable, Category = "Header Bob")
	float GetCurrentSanityPercent() const;

	// Take current bob type
	UFUNCTION(BlueprintCallable, Category = "Header Bob")
	EHeaderBobType GetCurrentBobType() const { return CurrentBobType; }

	// Check the entity
	UFUNCTION(BlueprintCallable, Category = "Header Bob")
	void UpdateEntityProximity();

private:
	// ==================== PRIVATE COMPONENTS ====================
	UPROPERTY()
	TObjectPtr<AEscapeITCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY()
	TObjectPtr<USanityComponent> SanityComponent;

	// ==================== PRIVATE VARIABLES ====================
	EHeaderBobType CurrentBobType;
	EHeaderBobType TargetBobType;

	float BobTimer;
	FVector OriginalCameraLocation;
	FVector CurrentCameraOffset;
	FVector TargetCameraOffset;

	float CurrentShakeIntensity;
	float TargetShakeIntensity;

	// Shock bob variables
	float CurrentShockIntensity;
	float ShockElapsedTime;
	bool bIsInShock;

	float DistanceToEntity;
	bool bIsEntityNear;

	UPROPERTY()
	TObjectPtr<UAudioComponent> HeartbeatAudioComponent;
	float HeartbeatTimer;

	// ==================== PRIVATE FUNCTIONS ====================
	void UpdateHeaderBobType();
	void UpdateCameraShake();
	void ApplyCameraBob(float DeltaTime);
	EHeaderBobType GetCurrentBobType(float CharacterSpeed) const;

	float GetSanityMultiplier(float SanityPercent) const;
	float GetFrequencyMultiplier(float SanityPercent) const;

	// Sanity event handler
	UFUNCTION()
	void OnSanityLevelChanged(ESanityLevel NewSanityLevel);

	// Audio functions
	void UpdateHeartbeatAudio(float SanityPercent);
	void PlayHeartbeat(float Volume);
	void StopHeartbeat();

	// Screen effects
	void ApplyScreenEffects(float SanityPercent);
	void UpdateVignette(float Intensity);
	void UpdateChromaticAberration(float Intensity);
	void ApplyScreenShake(float Intensity);

	// Shock effect
	void UpdateShockEffect(float DeltaTime);

	// Helper
	AActor* FindNearestEntity() const;
};
