// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LobbyCamera.generated.h"

class UCameraComponent;
class UPostProcessComponent;
class UAudioComponent;
class USoundBase;

UCLASS()
class ESCAPEIT_API ALobbyCamera : public APawn
{
	GENERATED_BODY()
public:
	ALobbyCamera();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UPostProcessComponent* PostProcess;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* AmbientAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* BreathingAudio;

	// Camera shake parameters
	UPROPERTY(EditAnywhere, Category = "Horror Effects")
	float BreathingIntensity = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects")
	float BreathingSpeed = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects")
	float DriftSpeed = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects")
	float DriftAmount = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects")
	float FOVMin = 58.0f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects")
	float FOVMax = 62.0f;

	// ============= NEW: Jump Scare System =============
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
	float JumpScareChance = 0.03f; // 3% mỗi giây

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
	float JumpScareMinInterval = 15.0f; // Tối thiểu 15s giữa các lần

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
	float JumpScareIntensity = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
	float JumpScareDuration = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Jump Scare")
	USoundBase* JumpScareSound; // Tiếng thì thầm hoặc tiếng động đột ngột

	// ============= NEW: Screen Glitch Effect =============
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Glitch")
	float GlitchFrequency = 0.02f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Glitch")
	float GlitchDuration = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Glitch")
	float GlitchMaxChromaticAberration = 1.5f;

	// ============= NEW: Screen Tearing/Distortion =============
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Distortion")
	bool bEnableScreenDistortion = true;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Distortion")
	float DistortionFrequency = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Distortion")
	float DistortionAmount = 0.02f;

	// ============= NEW: Pulse/Heartbeat Effect =============
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Heartbeat")
	bool bEnableHeartbeat = true;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Heartbeat")
	float HeartbeatSpeed = 1.2f; // BPM factor

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Heartbeat")
	float HeartbeatVignetteIntensity = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Heartbeat")
	USoundBase* HeartbeatSound;

	// ============= NEW: Shadow Flicker =============
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Lighting")
	bool bEnableShadowFlicker = true;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Lighting")
	float ShadowFlickerChance = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Lighting")
	float ShadowFlickerDuration = 0.2f;

	// ============= NEW: Whisper System =============
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	TArray<USoundBase*> RandomWhispers; // Array của các tiếng thì thầm

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	float WhisperChance = 0.008f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	float WhisperMinInterval = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	float WhisperVolume = 0.2f;

	// ============= NEW: Static Noise =============
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	USoundBase* StaticNoiseSound;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	float StaticNoiseBaseVolume = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	float StaticNoiseSpikeChance = 0.015f;

	// Post Process Settings
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
	float VignetteIntensity = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
	float Saturation = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
	float Contrast = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
	FLinearColor ColorGrading = FLinearColor(0.7f, 0.75f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
	float ChromaticAberration = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Post Process")
	float FilmGrain = 0.4f;

	// Audio Settings
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	USoundBase* AmbientWindSound;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	USoundBase* BreathingSound;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	float AmbientVolume = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	float BreathingVolume = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	bool bRandomBreathingPitch = true;

	// Runtime variables
	float TimeElapsed;
	FVector InitialLocation;
	FRotator InitialRotation;
	float InitialFOV;

	// NEW: Runtime states
	float LastJumpScareTime;
	float LastWhisperTime;
	bool bIsGlitching;
	float GlitchTimer;
	bool bIsShadowFlickering;
	float ShadowFlickerTimer;
	bool bIsJumpScaring;
	float JumpScareTimer;
	float BaseVignetteIntensity;
	float BaseChromaticAberration;

	// Camera movement functions
	void UpdateCameraBreathing(float DeltaTime);
	void UpdateCameraDrift(float DeltaTime);
	void UpdateFOVBreathing(float DeltaTime);

	// NEW: Horror effect functions
	void CheckForJumpScare(float DeltaTime);
	void TriggerJumpScare();
	void UpdateJumpScare(float DeltaTime);

	void CheckForGlitch(float DeltaTime);
	void TriggerGlitch();
	void UpdateGlitch(float DeltaTime);

	void UpdateScreenDistortion(float DeltaTime);
	void UpdateHeartbeatEffect(float DeltaTime);

	void CheckForShadowFlicker(float DeltaTime);
	void TriggerShadowFlicker();
	void UpdateShadowFlicker(float DeltaTime);

	void CheckForWhisper(float DeltaTime);
	void TriggerWhisper();

	void UpdateStaticNoise(float DeltaTime);

	void SetupPostProcessEffects();
	void SetupAudioEffects();
	void UpdateAudioEffects(float DeltaTime);
};