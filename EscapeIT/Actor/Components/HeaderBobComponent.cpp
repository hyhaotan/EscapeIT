// HeaderBobComponent.cpp
#include "HeaderBobComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EscapeIT/EscapeITCharacter.h"
#include "EscapeIT/Actor/Components/SanityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Perception/AIPerceptionComponent.h"

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

		// Reset post-process settings khi bắt đầu
		CameraComponent->PostProcessSettings.bOverride_VignetteIntensity = true;
		CameraComponent->PostProcessSettings.VignetteIntensity = 0.0f;
		CameraComponent->PostProcessBlendWeight = 0.0f;
	}

	SanityComponent = OwnerCharacter->SanityComponent;
	if (SanityComponent)
	{
		SanityComponent->OnSanityLevelChanged.AddDynamic(this, &UHeaderBobComponent::OnSanityLevelChanged);
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

	UpdateHeaderBobType();
	UpdateCameraShake();
	ApplyCameraBob(DeltaTime);
	UpdateShockEffect(DeltaTime);

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

	float CurrentSpeed = OwnerCharacter->GetCharacterMovement()->Velocity.Length();
	TargetBobType = GetCurrentBobType(CurrentSpeed);
}

EHeaderBobType UHeaderBobComponent::GetCurrentBobType(float CharacterSpeed) const
{
	if (CharacterSpeed < 50.0f)
	{
		return EHeaderBobType::Idle;
	}
	else
	{
		return EHeaderBobType::Walk;
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
		// Ngay cả idle, nếu Sanity rất thấp vẫn có chút vibration
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
		NewTargetIntensity = 0.5f * SanityMultiplier * EntityMultiplier;
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

	// Nếu idle và sanity cao, return to original
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

	TargetCameraOffset = FVector(0.0f, HorizontalBob, VerticalBob);
	CurrentCameraOffset = FMath::VInterpTo(
		CurrentCameraOffset,
		TargetCameraOffset,
		DeltaTime,
		TransitionSpeed * 2.0f
	);

	CameraComponent->SetRelativeLocation(OriginalCameraLocation + CurrentCameraOffset);
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
	// Tìm actors với tag "Entity" hoặc "Enemy"
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

	// Chỉ phát heartbeat khi Sanity < 50%
	if (SanityPercent < 50.0f)
	{
		float BaseRate = 60.0f; // BPM mặc định
		float MaxRate = 140.0f; // BPM tối đa khi Sanity < 30%

		// Calculate heartbeat rate
		float HeartRate = FMath::Lerp(BaseRate, MaxRate, (50.0f - SanityPercent) / 50.0f);
		float IntervalBetweenBeats = 60.0f / HeartRate; // Thời gian giữa các nhịp

		HeartbeatTimer += GetWorld()->DeltaTimeSeconds;

		if (HeartbeatTimer >= IntervalBetweenBeats)
		{
			float Volume = FMath::Lerp(0.2f, 1.0f, (50.0f - SanityPercent) / 50.0f) * HeartbeatVolumeMultiplier;
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

	// Setup audio component
	HeartbeatAudioComponent->SetSound(HeartbeatSFX);
	HeartbeatAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
	HeartbeatAudioComponent->bAutoActivate = false;
	HeartbeatAudioComponent->bAllowSpatialization = false;

	// Nếu chưa phát hoặc đã kết thúc, phát lại
	if (!HeartbeatAudioComponent->IsPlaying())
	{
		HeartbeatAudioComponent->Play(0.0f);
	}
	else
	{
		// Nếu đang phát, chỉ cập nhật volume
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

	// Tính intensity dựa trên sanity
	float EffectIntensity = 0.0f;
	if (SanityPercent < 70.0f)
	{
		EffectIntensity = (70.0f - SanityPercent) / 70.0f;
	}

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

	// Vignette effect - làm tối các cạnh màn hình khi sanity thấp
	PPSettings.bOverride_VignetteIntensity = true;
	PPSettings.VignetteIntensity = Intensity * 1.0f; // 0-1

	// Bloom effect để làm tăng cảm giác mềm mại và không tập trung
	PPSettings.bOverride_BloomIntensity = true;
	PPSettings.BloomIntensity = Intensity * 2.0f;

	// Color grading khi sanity thấp - colors trở nên nhạt đi
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

	// Chromatic aberration - tách màu RGB khi sanity thấp
	PPSettings.bOverride_ChromaticAberrationStartOffset = true;
	PPSettings.ChromaticAberrationStartOffset = Intensity * 0.5f;

	// Grain effect - thêm noise để tạo cảm giác rối
	PPSettings.bOverride_FilmGrainIntensity = true;
	PPSettings.FilmGrainIntensity = Intensity * 0.5f;

	// Screen tint - thay đổi màu sắc khi hoảng loạn
	PPSettings.bOverride_SceneColorTint = true;
	PPSettings.SceneColorTint = FLinearColor(
		1.0f - Intensity * 0.2f,  // Red - tăng hơn
		1.0f - Intensity * 0.1f,  // Green
		1.0f - Intensity * 0.3f   // Blue - tăng hơn để tạo cảm giác lạnh
	);

	CameraComponent->PostProcessBlendWeight = 1.0f;
}

void UHeaderBobComponent::ApplyScreenShake(float Intensity)
{
	if (!GetWorld() || !OwnerCharacter || !CameraComponent)
	{
		return;
	}

	// Apply lightweight screen shake thông qua camera post-process
	// Thay vì dùng CameraShake (quá mạnh), dùng FOV wobble để subtle hơn

	float ShakeScale = Intensity * ScreenShakeIntensity;

	if (ShakeScale > 0.01f)
	{
		FPostProcessSettings& PPSettings = CameraComponent->PostProcessSettings;

		// Lens distortion khi entity gần
		PPSettings.bOverride_LensFlareIntensity = true;
		PPSettings.LensFlareIntensity = ShakeScale * 0.3f;

		// Slight FOV wobble
		float CurrentFOV = CameraComponent->FieldOfView;
		float FOVWobble = 70.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * ShakeScale * 2.0f;
		CameraComponent->SetFieldOfView(FOVWobble);

		CameraComponent->PostProcessBlendWeight = 1.0f;
	}
}

// ==================== SANITY EVENTS ====================

void UHeaderBobComponent::OnSanityLevelChanged(ESanityLevel NewSanityLevel)
{
	switch (NewSanityLevel)
	{
	case ESanityLevel::High:
		// Sanity cao: reset effects
		CurrentShockIntensity = 0.0f;
		HeartbeatTimer = 0.0f;
		break;

	case ESanityLevel::Medium:
		// Sanity vừa: không cần action đặc biệt
		break;

	case ESanityLevel::Low:
		// Sanity thấp: reset timer để tạo hiệu ứng mới
		BobTimer = 0.0f;
		break;

	case ESanityLevel::Critical:
		// Sanity rất thấp: shock effect bùng nổ
		BobTimer = 0.0f;
		break;
	}
}

void UHeaderBobComponent::OnSanityRecovered()
{
	// Bob sẽ từ từ trở lại bình thường qua UpdateCameraShake()
	// Có thể thêm hiệu ứng phục hồi ở đây nếu cần
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