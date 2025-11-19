
#include "HeaderBobComponent.h"

#include "StaminaComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EscapeIT/EscapeITCharacter.h"
#include "EscapeIT/Actor/Components/SanityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

UHeaderBobComponent::UHeaderBobComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;

	BobTimer = 0.0f;
	CurrentBobType = EHeaderBobType::Idle;
	TargetBobType = EHeaderBobType::Idle;
	CurrentShakeIntensity = 0.0f;
	TargetShakeIntensity = 0.0f;

	CurrentShockIntensity = 0.0f;
	ShockElapsedTime = 0.0f;
	bIsInShock = false;

	DistanceToEntity = 10000.0f;
	bIsEntityNear = false;

	HeartbeatTimer = 0.0f;
	BreathingTimer = 0.0f;

	CurrentFOV = DefaultFOV;
	TargetFOV = DefaultFOV;
	CurrentCameraTilt = 0.0f;
	TargetCameraTilt = 0.0f;

	bIsLandingImpactActive = false;
	LandingImpactIntensity = 0.0f;
}

void UHeaderBobComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AEscapeITCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("HeaderBobComponent must be attached to AEscapeITCharacter!"));
		return;
	}

	CameraComponent = OwnerCharacter->FirstPersonCameraComponent;
	if (CameraComponent)
	{
		OriginalCameraLocation = CameraComponent->GetRelativeLocation();
		OriginalCameraRotation = CameraComponent->GetRelativeRotation();
		CurrentFOV = CameraComponent->FieldOfView;
		DefaultFOV = CurrentFOV;

		// Reset post-process settings
		CameraComponent->PostProcessSettings.bOverride_VignetteIntensity = true;
		CameraComponent->PostProcessSettings.VignetteIntensity = 0.0f;
		CameraComponent->PostProcessBlendWeight = 0.0f;
	}

	SanityComponent = OwnerCharacter->SanityComponent;
	if (SanityComponent)
	{
		SanityComponent->OnSanityLevelChanged.AddDynamic(this, &UHeaderBobComponent::OnSanityLevelChanged);
	}
	
	StaminaComponent = OwnerCharacter->FindComponentByClass<UStaminaComponent>();
	if (!StaminaComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("HeaderBobComponent: StaminaComponent not found!"));
	}

	// Setup heartbeat audio component
	if (bEnableHeartbeatAudio && HeartbeatSFX)
	{
		HeartbeatAudioComponent = NewObject<UAudioComponent>(this);
		HeartbeatAudioComponent->RegisterComponent();
		HeartbeatAudioComponent->AttachToComponent(CameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
		HeartbeatAudioComponent->bAutoActivate = false;
	}
}

void UHeaderBobComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || !CameraComponent || !SanityComponent)
	{
		return;
	}

	float CurrentSanity = GetCurrentSanityPercent();

	// Core updates
	UpdateHeaderBobType();
	UpdateCameraShake();
	ApplyCameraBob(DeltaTime);
	UpdateShockEffect(DeltaTime);

	// New features
	UpdateFOVDynamics(DeltaTime);
	UpdateCameraTilt(DeltaTime);
	UpdateBreathing(DeltaTime);
	UpdateLandingImpact(DeltaTime);

	if (bEnableEntityProximity)
	{
		UpdateEntityProximity();
	}

	UpdateHeartbeatAudio(CurrentSanity);
	ApplyScreenEffects(CurrentSanity);
}

// ==================== HEADER BOB CORE ====================

void UHeaderBobComponent::UpdateHeaderBobType()
{
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		return;
	}

	float CurrentSpeed = OwnerCharacter->GetCharacterMovement()->Velocity.Size2D();
	TargetBobType = GetCurrentBobType(CurrentSpeed);
}

EHeaderBobType UHeaderBobComponent::GetCurrentBobType(float CharacterSpeed) const
{
	if (CharacterSpeed < 50.0f)
	{
		return EHeaderBobType::Idle;
	}
	else if (CharacterSpeed < 400.0f) // Walk threshold
	{
		return EHeaderBobType::Walk;
	}
	else // Sprint threshold
	{
		return EHeaderBobType::Sprint;
	}
}

void UHeaderBobComponent::UpdateCameraShake()
{
	float CurrentSanity = GetCurrentSanityPercent();
	float SanityMultiplier = GetSanityMultiplier(CurrentSanity);

	// Smooth transition giữa bob types
	if (CurrentBobType != TargetBobType)
	{
		CurrentBobType = TargetBobType;
		BobTimer = 0.0f;
	}

	// Entity proximity modifier
	float EntityMultiplier = bIsEntityNear ? EntityProximityMultiplier : 1.0f;

	// Update target shake intensity
	float NewTargetIntensity = 0.0f;
	switch (CurrentBobType)
	{
	case EHeaderBobType::Idle:
		if (CurrentSanity < IdleVibrationThreshold && bEnableIdleVibration)
		{
			NewTargetIntensity = 0.3f * SanityMultiplier * EntityMultiplier;
		}
		else
		{
			NewTargetIntensity = 0.0f;
		}
		break;

	case EHeaderBobType::Walk:
		NewTargetIntensity = 0.6f * SanityMultiplier * EntityMultiplier;
		break;

	case EHeaderBobType::Sprint:
		NewTargetIntensity = 0.9f * SanityMultiplier * EntityMultiplier;
		break;
	}

	TargetShakeIntensity = NewTargetIntensity;
	CurrentShakeIntensity = FMath::FInterpTo(
		CurrentShakeIntensity,
		TargetShakeIntensity,
		GetWorld()->DeltaTimeSeconds,
		TransitionSpeed
	);
}

void UHeaderBobComponent::ApplyCameraBob(float DeltaTime)
{
	if (!CameraComponent)
	{
		return;
	}

	float CurrentSanity = GetCurrentSanityPercent();
	float SanityMultiplier = GetSanityMultiplier(CurrentSanity);
	float FrequencyMultiplier = GetFrequencyMultiplier(CurrentSanity);
	float EntityMultiplier = bIsEntityNear ? EntityProximityMultiplier : 1.0f;

	// Nếu idle và sanity cao, chỉ apply breathing
	if (CurrentBobType == EHeaderBobType::Idle && CurrentSanity >= IdleVibrationThreshold)
	{
		CurrentCameraOffset = FMath::VInterpTo(
			CurrentCameraOffset,
			FVector::ZeroVector,
			DeltaTime,
			TransitionSpeed
		);
		CameraComponent->SetRelativeLocation(OriginalCameraLocation + CurrentCameraOffset);
		return;
	}

	BobTimer += DeltaTime;

	float Amplitude = 0.0f;
	float Frequency = 0.0f;

	switch (CurrentBobType)
	{
	case EHeaderBobType::Walk:
		Amplitude = WalkAmplitude * SanityMultiplier * EntityMultiplier;
		Frequency = WalkFrequency * FrequencyMultiplier * EntityMultiplier;
		break;

	case EHeaderBobType::Sprint:
		Amplitude = SprintAmplitude * SanityMultiplier * EntityMultiplier;
		Frequency = SprintFrequency * FrequencyMultiplier * EntityMultiplier;
		break;

	case EHeaderBobType::Idle:
		if (CurrentSanity < IdleVibrationThreshold && bEnableIdleVibration)
		{
			Amplitude = IdleVibrationAmplitude * SanityMultiplier * EntityMultiplier;
			Frequency = IdleVibrationFrequency * FrequencyMultiplier * EntityMultiplier;
		}
		else
		{
			Amplitude = IdleAmplitude;
			Frequency = IdleFrequency;
		}
		break;
	}

	// Calculate bob offset
	float VerticalBob = FMath::Sin(BobTimer * Frequency * PI) * Amplitude * CurrentShakeIntensity;
	float HorizontalBob = FMath::Cos(BobTimer * Frequency * PI * 0.5f) * Amplitude * CurrentShakeIntensity * 0.5f;

	// Add shock effect
	VerticalBob += FMath::Sin(BobTimer * 8.0f) * CurrentShockIntensity * 0.05f;
	HorizontalBob += FMath::Cos(BobTimer * 8.0f) * CurrentShockIntensity * 0.03f;

	// Add landing impact
	if (bIsLandingImpactActive)
	{
		VerticalBob += -LandingImpactIntensity * 5.0f;
	}

	TargetCameraOffset = FVector(0.0f, HorizontalBob, VerticalBob);
	CurrentCameraOffset = FMath::VInterpTo(
		CurrentCameraOffset,
		TargetCameraOffset,
		DeltaTime,
		TransitionSpeed * 2.0f
	);

	CameraComponent->SetRelativeLocation(OriginalCameraLocation + CurrentCameraOffset);
}

// ==================== FOV DYNAMICS ====================

void UHeaderBobComponent::UpdateFOVDynamics(float DeltaTime)
{
	if (!bEnableFOVDynamics || !CameraComponent)
	{
		return;
	}

	// Determine target FOV based on movement state
	switch (CurrentBobType)
	{
	case EHeaderBobType::Idle:
		TargetFOV = DefaultFOV;
		break;

	case EHeaderBobType::Walk:
		TargetFOV = WalkFOV;
		break;

	case EHeaderBobType::Sprint:
		TargetFOV = SprintFOV;
		break;
	}

	// Apply landing FOV punch
	if (bIsLandingImpactActive)
	{
		TargetFOV += LandingFOVPunch * LandingImpactIntensity;
	}

	// Smoothly interpolate to target FOV
	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed);
	CameraComponent->SetFieldOfView(CurrentFOV);
}

// ==================== CAMERA TILT ====================

void UHeaderBobComponent::UpdateCameraTilt(float DeltaTime)
{
	if (!bEnableCameraTilt || !OwnerCharacter || !CameraComponent)
	{
		return;
	}

	// Get horizontal velocity (strafe)
	FVector Velocity = OwnerCharacter->GetVelocity();
	FVector RightVector = OwnerCharacter->GetActorRightVector();
	float RightVelocity = FVector::DotProduct(Velocity, RightVector);

	// Calculate target tilt based on strafe speed
	TargetCameraTilt = -RightVelocity * TiltAmount * 0.01f;
	TargetCameraTilt = FMath::Clamp(TargetCameraTilt, -MaxTiltAngle, MaxTiltAngle);

	// Smooth interpolation
	CurrentCameraTilt = FMath::FInterpTo(CurrentCameraTilt, TargetCameraTilt, DeltaTime, TiltInterpSpeed);

	// Apply tilt to camera rotation
	FRotator NewRotation = OriginalCameraRotation;
	NewRotation.Roll += CurrentCameraTilt;
	CameraComponent->SetRelativeRotation(NewRotation);
}

// ==================== BREATHING ====================

void UHeaderBobComponent::UpdateBreathing(float DeltaTime)
{
	if (!bEnableBreathing || !CameraComponent || CurrentBobType != EHeaderBobType::Idle)
	{
		return;
	}

	float CurrentSanity = GetCurrentSanityPercent();

	// NEW: Get breathing intensity from StaminaComponent
	float BreathMultiplier = 1.0f;
	
	// Combine sanity and stamina effects
	if (CurrentSanity < 50.0f)
	{
		// Sanity effect on breathing
		BreathMultiplier = FMath::Lerp(1.0f, StaminaBreathingMultiplier, (50.0f - CurrentSanity) / 50.0f);
	}

	if (StaminaComponent)
	{
		// Stamina effect on breathing (multiplicative)
		float StaminaBreathIntensity = StaminaComponent->GetBreathingIntensity();
		BreathMultiplier *= StaminaBreathIntensity;
	}

	BreathingTimer += DeltaTime * BreathMultiplier;

	// Calculate breathing offset
	float BreathOffset = FMath::Sin(BreathingTimer * BreathingFrequency * PI * 2.0f) * BreathingAmplitude;

	// Apply subtle breathing to camera
	FVector BreathLocation = OriginalCameraLocation;
	BreathLocation.Z += BreathOffset;

	// Only apply if not moving
	if (CurrentBobType == EHeaderBobType::Idle && CurrentSanity >= IdleVibrationThreshold)
	{
		CameraComponent->SetRelativeLocation(BreathLocation);
	}
}

// ==================== LANDING IMPACT ====================

void UHeaderBobComponent::TriggerLandingImpact(float FallSpeed)
{
	if (!bEnableLandingImpact || FallSpeed < LandingImpactThreshold)
	{
		return;
	}

	// Calculate impact intensity based on fall speed
	float ImpactScale = FMath::Clamp((FallSpeed - LandingImpactThreshold) / 600.0f, 0.0f, 1.0f);
	
	bIsLandingImpactActive = true;
	LandingImpactIntensity = ImpactScale * LandingImpactStrength;

	// Trigger camera shake
	if (LandingCameraShakeClass && OwnerCharacter)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			PC->ClientStartCameraShake(LandingCameraShakeClass, ImpactScale);
		}
	}

	// Reset bob timer for impact effect
	BobTimer = 0.0f;
}

void UHeaderBobComponent::UpdateLandingImpact(float DeltaTime)
{
	if (!bIsLandingImpactActive)
	{
		return;
	}

	// Decay landing impact over time
	LandingImpactIntensity = FMath::FInterpTo(LandingImpactIntensity, 0.0f, DeltaTime, 5.0f);

	if (LandingImpactIntensity < 0.01f)
	{
		bIsLandingImpactActive = false;
		LandingImpactIntensity = 0.0f;
	}
}

// ==================== SANITY MULTIPLIERS ====================

float UHeaderBobComponent::GetSanityMultiplier(float SanityPercent) const
{
	if (SanityPercent >= 70.0f)
	{
		return 1.0f;
	}
	else if (SanityPercent >= 50.0f)
	{
		return SanityMultiplier_Medium;
	}
	else if (SanityPercent >= 30.0f)
	{
		return SanityMultiplier_Low;
	}
	else
	{
		return SanityMultiplier_Critical;
	}
}

float UHeaderBobComponent::GetFrequencyMultiplier(float SanityPercent) const
{
	if (SanityPercent >= 50.0f)
	{
		return 1.0f;
	}
	else if (SanityPercent >= 30.0f)
	{
		return FrequencyMultiplier_Low;
	}
	else
	{
		return FrequencyMultiplier_Critical;
	}
}

// ==================== SHOCK EFFECT ====================

void UHeaderBobComponent::TriggerShockBob()
{
	bIsInShock = true;
	CurrentShockIntensity = ShockIntensity;
	ShockElapsedTime = 0.0f;
	BobTimer = 0.0f;

	// Trigger FOV punch for shock
	if (CameraComponent)
	{
		CurrentFOV = DefaultFOV - 15.0f;
	}
}

void UHeaderBobComponent::UpdateShockEffect(float DeltaTime)
{
	if (bIsInShock)
	{
		ShockElapsedTime += DeltaTime;

		// Decay shock over time
		CurrentShockIntensity = FMath::Lerp(
			ShockIntensity,
			0.0f,
			ShockElapsedTime / ShockDuration
		);

		if (ShockElapsedTime >= ShockDuration)
		{
			bIsInShock = false;
			CurrentShockIntensity = 0.0f;
		}
	}
}

// ==================== ENTITY PROXIMITY ====================

void UHeaderBobComponent::UpdateEntityProximity()
{
	AActor* NearestEntity = FindNearestEntity();

	if (NearestEntity)
	{
		DistanceToEntity = FVector::Dist(OwnerCharacter->GetActorLocation(), NearestEntity->GetActorLocation());
		bIsEntityNear = DistanceToEntity < EntityProximityThreshold;
	}
	else
	{
		DistanceToEntity = 10000.0f;
		bIsEntityNear = false;
	}
}

AActor* UHeaderBobComponent::FindNearestEntity() const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Entity"), FoundActors);

	if (FoundActors.IsEmpty())
	{
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FoundActors);
	}

	AActor* NearestActor = nullptr;
	float MinDistance = FLT_MAX;

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor != OwnerCharacter)
		{
			float Distance = FVector::Dist(OwnerCharacter->GetActorLocation(), Actor->GetActorLocation());
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				NearestActor = Actor;
			}
		}
	}

	return NearestActor;
}

// ==================== AUDIO FUNCTIONS ====================

void UHeaderBobComponent::UpdateHeartbeatAudio(float SanityPercent)
{
	if (!bEnableHeartbeatAudio || !HeartbeatSFX || !HeartbeatAudioComponent)
	{
		return;
	}

	// Check if should play heartbeat (sanity OR stamina low)
	bool bShouldPlayHeartbeat = SanityPercent < 50.0f;
	
	// NEW: Also play heartbeat when stamina is low
	if (StaminaComponent && StaminaComponent->GetStaminaPercent() < 0.3f)
	{
		bShouldPlayHeartbeat = true;
	}

	if (bShouldPlayHeartbeat)
	{
		float BaseRate = 60.0f;
		float MaxRate = 140.0f;

		// Calculate heart rate from sanity
		float SanityHeartRate = FMath::Lerp(BaseRate, MaxRate, (50.0f - SanityPercent) / 50.0f);

		// NEW: Calculate heart rate from stamina
		float StaminaHeartRate = BaseRate;
		if (StaminaComponent)
		{
			float StaminaPercent = StaminaComponent->GetStaminaPercent() * 100.0f;
			if (StaminaPercent < 30.0f)
			{
				StaminaHeartRate = FMath::Lerp(BaseRate, MaxRate * 0.8f, (30.0f - StaminaPercent) / 30.0f);
			}
		}

		// Use the higher heart rate
		float HeartRate = FMath::Max(SanityHeartRate, StaminaHeartRate);
		float IntervalBetweenBeats = 60.0f / HeartRate;

		HeartbeatTimer += GetWorld()->DeltaTimeSeconds;

		if (HeartbeatTimer >= IntervalBetweenBeats)
		{
			// Calculate volume
			float SanityVolume = FMath::Lerp(0.2f, 1.0f, (50.0f - SanityPercent) / 50.0f);
			float StaminaVolume = 0.2f;
			if (StaminaComponent)
			{
				float StaminaPercent = StaminaComponent->GetStaminaPercent();
				if (StaminaPercent < 0.3f)
				{
					StaminaVolume = FMath::Lerp(0.8f, 0.2f, StaminaPercent / 0.3f);
				}
			}

			float Volume = FMath::Max(SanityVolume, StaminaVolume) * HeartbeatVolumeMultiplier;
			PlayHeartbeat(Volume);
			HeartbeatTimer = 0.0f;
		}
	}
	else
	{
		StopHeartbeat();
	}
}

void UHeaderBobComponent::PlayHeartbeat(float Volume)
{
	if (!HeartbeatAudioComponent || !HeartbeatSFX)
	{
		return;
	}

	HeartbeatAudioComponent->SetSound(HeartbeatSFX);
	HeartbeatAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
	HeartbeatAudioComponent->bAutoActivate = false;
	HeartbeatAudioComponent->bAllowSpatialization = false;

	if (!HeartbeatAudioComponent->IsPlaying())
	{
		HeartbeatAudioComponent->Play(0.0f);
	}
	else
	{
		HeartbeatAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
	}
}

void UHeaderBobComponent::StopHeartbeat()
{
	if (HeartbeatAudioComponent)
	{
		if (HeartbeatAudioComponent->IsPlaying())
		{
			HeartbeatAudioComponent->FadeOut(0.1f, 0.0f);
		}
		HeartbeatTimer = 0.0f;
	}
}

// ==================== SCREEN EFFECTS ====================

void UHeaderBobComponent::ApplyScreenEffects(float SanityPercent)
{
	if (!bEnableVignette && !bEnableChromaticAberration && !bEnableScreenShake)
	{
		return;
	}

	// Calculate effect intensity from sanity
	float SanityIntensity = 0.0f;
	if (SanityPercent < 70.0f)
	{
		SanityIntensity = (70.0f - SanityPercent) / 70.0f;
	}

	// NEW: Add stamina effect
	float StaminaIntensity = 0.0f;
	if (StaminaComponent)
	{
		float StaminaPercent = StaminaComponent->GetStaminaPercent();
		if (StaminaPercent < 0.3f) // Low stamina
		{
			StaminaIntensity = (0.3f - StaminaPercent) / 0.3f * 0.5f; // Max 0.5 intensity
		}
	}

	// Combine effects (take max)
	float EffectIntensity = FMath::Max(SanityIntensity, StaminaIntensity);

	if (bEnableVignette)
	{
		UpdateVignette(EffectIntensity);
	}

	if (bEnableChromaticAberration)
	{
		UpdateChromaticAberration(EffectIntensity);
	}

	if (bEnableScreenShake && bIsEntityNear)
	{
		ApplyScreenShake(EffectIntensity);
	}
}

void UHeaderBobComponent::UpdateVignette(float Intensity)
{
	if (!CameraComponent)
	{
		return;
	}

	FPostProcessSettings& PPSettings = CameraComponent->PostProcessSettings;

	PPSettings.bOverride_VignetteIntensity = true;
	PPSettings.VignetteIntensity = Intensity * 1.0f;

	PPSettings.bOverride_BloomIntensity = true;
	PPSettings.BloomIntensity = Intensity * 2.0f;

	PPSettings.bOverride_ColorSaturation = true;
	PPSettings.ColorSaturation = FVector4(1.0f - Intensity * 0.5f, 1.0f, 1.0f, 1.0f);

	CameraComponent->PostProcessBlendWeight = 1.0f;
}

void UHeaderBobComponent::UpdateChromaticAberration(float Intensity)
{
	if (!CameraComponent)
	{
		return;
	}

	FPostProcessSettings& PPSettings = CameraComponent->PostProcessSettings;

	PPSettings.bOverride_ChromaticAberrationStartOffset = true;
	PPSettings.ChromaticAberrationStartOffset = Intensity * 0.5f;

	PPSettings.bOverride_FilmGrainIntensity = true;
	PPSettings.FilmGrainIntensity = Intensity * 0.5f;

	PPSettings.bOverride_SceneColorTint = true;
	PPSettings.SceneColorTint = FLinearColor(
		1.0f - Intensity * 0.2f,
		1.0f - Intensity * 0.1f,
		1.0f - Intensity * 0.3f
	);

	CameraComponent->PostProcessBlendWeight = 1.0f;
}

void UHeaderBobComponent::ApplyScreenShake(float Intensity)
{
	if (!GetWorld() || !OwnerCharacter || !CameraComponent)
	{
		return;
	}

	float ShakeScale = Intensity * ScreenShakeIntensity;

	if (ShakeScale > 0.01f)
	{
		FPostProcessSettings& PPSettings = CameraComponent->PostProcessSettings;

		PPSettings.bOverride_LensFlareIntensity = true;
		PPSettings.LensFlareIntensity = ShakeScale * 0.3f;

		CameraComponent->PostProcessBlendWeight = 1.0f;
	}
}

// ==================== SANITY EVENTS ====================

void UHeaderBobComponent::OnSanityLevelChanged(ESanityLevel NewSanityLevel)
{
	switch (NewSanityLevel)
	{
	case ESanityLevel::High:
		CurrentShockIntensity = 0.0f;
		HeartbeatTimer = 0.0f;
		break;

	case ESanityLevel::Medium:
		break;

	case ESanityLevel::Low:
		BobTimer = 0.0f;
		break;

	case ESanityLevel::Critical:
		BobTimer = 0.0f;
		TriggerShockBob();
		break;
	}
}

void UHeaderBobComponent::OnSanityRecovered()
{
	// Bob will gradually return to normal through UpdateCameraShake()
}

// ==================== GETTERS ====================

float UHeaderBobComponent::GetCurrentSanityPercent() const
{
	if (SanityComponent)
	{
		return SanityComponent->GetSanityPercent() * 100.0f;
	}
	return 100.0f;
}