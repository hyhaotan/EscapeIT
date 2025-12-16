// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/WindowJumpscareActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "EscapeITCharacter.h"
#include "UI/SanityWidget.h"
#include "UI/HUD/WidgetManager.h"
#include "Actor/Components/SanityComponent.h"

AWindowJumpscareActor::AWindowJumpscareActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup components
	WindowFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Window Frame"));
	RootComponent = WindowFrameMesh;

	LeftWindowHinge = CreateDefaultSubobject<USceneComponent>(TEXT("Left Window Hinge"));
	LeftWindowHinge->SetupAttachment(RootComponent);

	LeftWindowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Window"));
	LeftWindowMesh->SetupAttachment(LeftWindowHinge);

	RightWindowHinge = CreateDefaultSubobject<USceneComponent>(TEXT("Right Window Hinge"));
	RightWindowHinge->SetupAttachment(RootComponent);

	RightWindowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Window"));
	RightWindowMesh->SetupAttachment(RightWindowHinge);

	GhostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ghost Actor"));
	GhostMesh->SetupAttachment(RootComponent);
	GhostMesh->SetVisibility(false);

	BoxCollission = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collission"));
	BoxCollission->SetupAttachment(RootComponent);
	BoxCollission->OnComponentBeginOverlap.AddDynamic(this, &AWindowJumpscareActor::OnOverlapBegin);

	GhostSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Ghost Spring Arm"));
	GhostSpringArm->SetupAttachment(GhostMesh, FName("head"));

	GhostCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Ghost Camera"));
	GhostCamera->SetupAttachment(GhostSpringArm);

	WindowTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Window Timeline"));

	// NEW: Flicker Light
	FlickerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Flicker Light"));
	FlickerLight->SetupAttachment(RootComponent);
	FlickerLight->SetVisibility(false);
	FlickerLight->SetIntensity(0.0f);

	// NEW: Heartbeat Audio Component
	HeartbeatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Heartbeat Audio"));
	HeartbeatAudioComponent->SetupAttachment(RootComponent);
	HeartbeatAudioComponent->bAutoActivate = false;
}

void AWindowJumpscareActor::BeginPlay()
{
	Super::BeginPlay();

	LeftHingeInitialRot = LeftWindowHinge->GetRelativeRotation();
	RightHingeInitialRot = RightWindowHinge->GetRelativeRotation();

	TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerCon)
	{
		WidgetManager = Cast<AWidgetManager>(PlayerCon->GetHUD());
		if (!WidgetManager)
		{
			UE_LOG(LogTemp, Error, TEXT("WidgetManager not found!"));
		}

		TObjectPtr<AEscapeITCharacter> PlayerCharacter = Cast<AEscapeITCharacter>(PlayerCon->GetPawn());
		if (PlayerCharacter)
		{
			SanityComponent = PlayerCharacter->FindComponentByClass<USanityComponent>();
			if (!SanityComponent)
			{
				UE_LOG(LogTemp, Error, TEXT("SanityComponent not found on player!"));
			}
		}
	}

	if (WindowTimeline && WindowCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("UpdateWindowRotation"));
		WindowTimeline->AddInterpFloat(WindowCurve, TimelineCallback);

		WindowTimeline->SetLooping(false);
		WindowTimeline->SetPlayRate(1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WindowTimeline or WindowCurve is not set!"));
	}

	// Setup Heartbeat Audio
	if (HeartbeatSound && HeartbeatAudioComponent)
	{
		HeartbeatAudioComponent->SetSound(HeartbeatSound);
		HeartbeatAudioComponent->SetVolumeMultiplier(0.0f);
	}

	// Setup Flicker Light
	if (FlickerLight && bUseFlickerEffect)
	{
		FlickerLight->SetLightColor(FlickerColor);
	}
}

void AWindowJumpscareActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update heartbeat based on distance
	if (!bHasTriggered && HeartbeatSound)
	{
		UpdateHeartbeat(DeltaTime);
	}
}

void AWindowJumpscareActor::UpdateHeartbeat(float DeltaTime)
{
	TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerCon)
		return;

	AActor* PlayerPawn = PlayerCon->GetPawn();
	if (!PlayerPawn)
		return;

	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	if (Distance <= HeartbeatTriggerDistance)
	{
		// Calculate volume based on distance (closer = louder)
		float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / HeartbeatTriggerDistance), 0.0f, 1.0f);
		VolumeMultiplier *= HeartbeatMaxVolume;

		if (HeartbeatAudioComponent && !HeartbeatAudioComponent->IsPlaying())
		{
			HeartbeatAudioComponent->Play();
		}

		if (HeartbeatAudioComponent)
		{
			HeartbeatAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
		}

		bIsPlayerNearby = true;
	}
	else
	{
		if (HeartbeatAudioComponent && HeartbeatAudioComponent->IsPlaying())
		{
			HeartbeatAudioComponent->Stop();
		}
		bIsPlayerNearby = false;
	}
}

float AWindowJumpscareActor::CalculateJumpscareIntensity(float Distance)
{
	if (!bUseDistanceScaling)
		return 1.0f;

	// Closer = higher intensity (1.0 to 2.0)
	float Intensity = FMath::GetMappedRangeValueClamped(
		FVector2D(MaxIntensityDistance, MinIntensityDistance),
		FVector2D(2.0f, 1.0f),
		Distance
	);

	return Intensity;
}

void AWindowJumpscareActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasTriggered)
		return;

	APlayerController* PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerCon)
		return;

	AActor* PlayerPawn = PlayerCon->GetPawn();
	if (PlayerPawn == OtherActor)
	{
		bHasTriggered = true;

		// Stop heartbeat
		if (HeartbeatAudioComponent && HeartbeatAudioComponent->IsPlaying())
		{
			HeartbeatAudioComponent->Stop();
		}

		// Calculate intensity based on distance
		float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
		CurrentJumpscareIntensity = CalculateJumpscareIntensity(Distance);

		TriggerJumpscare();
	}
}

void AWindowJumpscareActor::UpdateWindowRotation(float Value)
{
	FRotator NewLeftRot = LeftHingeInitialRot;
	NewLeftRot.Yaw += LeftWindowOpenAngle * Value;
	LeftWindowHinge->SetRelativeRotation(NewLeftRot);

	FRotator NewRightRot = RightHingeInitialRot;
	NewRightRot.Yaw -= RightWindowOpenAngle * Value;
	RightWindowHinge->SetRelativeRotation(NewRightRot);
}

void AWindowJumpscareActor::TriggerJumpscare()
{
	if (!GhostCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("GhostCamera is not set!"));
		return;
	}

	TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerCon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't get the player controller"));
		return;
	}

	DisableInput(PlayerCon);
	OriginalViewTarget = PlayerCon->GetViewTarget();

	// Select random ghost if enabled
	if (bUseRandomGhost && GhostMeshVariations.Num() > 0)
	{
		SelectRandomGhost();
	}

	if (bUseMultiStage)
	{
		// Multi-stage jumpscare
		GetWorldTimerManager().SetTimer(Stage1Timer, this, &AWindowJumpscareActor::ExecuteStage1, Stage1Delay, false);
	}
	else
	{
		// Single-stage jumpscare (original behavior)
		ExecuteFinalStage();
	}
}

void AWindowJumpscareActor::ExecuteStage1()
{
	// Stage 1: Window starts to open slightly
	if (WindowTimeline)
	{
		WindowTimeline->PlayFromStart();
		// Stop at Stage1WindowOpenPercent
		float StopTime = WindowTimeline->GetTimelineLength() * Stage1WindowOpenPercent;
		GetWorldTimerManager().SetTimer(Stage2Timer, this, &AWindowJumpscareActor::ExecuteStage2, StopTime, false);
	}

	if (WindowOpenSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WindowOpenSound, GetActorLocation(), 0.5f);
	}
}

void AWindowJumpscareActor::ExecuteStage2()
{
	// Stage 2: Pause briefly, then continue
	WindowTimeline->Stop();

	// Wait for Stage2Delay, then execute final stage
	GetWorldTimerManager().SetTimer(Stage2Timer, this, &AWindowJumpscareActor::ExecuteFinalStage, Stage2Delay, false);
}

void AWindowJumpscareActor::ExecuteFinalStage()
{
	TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerCon)
		return;

	// Camera switch
	PlayerCon->SetViewTargetWithBlend(this, 1.0f, EViewTargetBlendFunction::VTBlend_EaseInOut, 1.0f, false);

	// Window animation
	if (WindowTimeline)
	{
		if (bUseMultiStage)
		{
			WindowTimeline->Play(); // Continue from where it stopped
		}
		else
		{
			WindowTimeline->PlayFromStart();
		}
	}

	// Show ghost
	if (GhostMesh)
	{
		GhostMesh->SetVisibility(true);
	}

	// Sound effects
	if (WindowOpenSound && !bUseMultiStage)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WindowOpenSound, GetActorLocation());
	}

	// Random jumpscare sound
	USoundBase* SelectedSound = GetRandomJumpscareSound();
	if (SelectedSound)
	{
		float VolumeMultiplier = FMath::Clamp(CurrentJumpscareIntensity, 0.8f, 1.5f);
		UGameplayStatics::PlaySoundAtLocation(this, SelectedSound, GetActorLocation(), VolumeMultiplier);
	}

	// Apply camera shake
	ApplyCameraShake();

	// Apply slow motion
	if (bUseSlowMotion)
	{
		ApplySlowMotion();
	}

	// Spawn particle effects
	SpawnParticleEffects();

	// Trigger flicker effect
	if (bUseFlickerEffect)
	{
		TriggerFlickerEffect();
	}

	// Reduce sanity based on intensity
	if (SanityComponent)
	{
		float ReductionAmount = FMath::Lerp(BaseSanityReduction, MaxSanityReduction, CurrentJumpscareIntensity - 1.0f);
		SanityComponent->ReduceSanity(ReductionAmount);
	}

	// Schedule window close
	GetWorldTimerManager().SetTimer(CloseWindowTimer, this, &AWindowJumpscareActor::CloseWindows, CloseDelayTime, false);
}

void AWindowJumpscareActor::ApplyCameraShake()
{
	if (JumpscareCameraShake)
	{
		TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
		if (PlayerCon)
		{
			float ShakeScale = CameraShakeScale * CurrentJumpscareIntensity;
			PlayerCon->ClientStartCameraShake(JumpscareCameraShake, ShakeScale);
		}
	}
}

void AWindowJumpscareActor::ApplySlowMotion()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMotionScale);

	// Restore time scale after duration
	GetWorldTimerManager().SetTimer(
		SlowMotionTimer,
		this,
		&AWindowJumpscareActor::RestoreTimeScale,
		SlowMotionDuration * SlowMotionScale, // Adjust for slow motion
		false
	);
}

void AWindowJumpscareActor::RestoreTimeScale()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

void AWindowJumpscareActor::SpawnParticleEffects()
{
	if (GhostAppearEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			GhostAppearEffect,
			GhostMesh->GetComponentLocation(),
			FRotator::ZeroRotator,
			true
		);
	}

	if (WindowBurstEffect)
	{
		// Spawn at left window
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			WindowBurstEffect,
			LeftWindowMesh->GetComponentLocation(),
			FRotator::ZeroRotator,
			true
		);

		// Spawn at right window
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			WindowBurstEffect,
			RightWindowMesh->GetComponentLocation(),
			FRotator::ZeroRotator,
			true
		);
	}
}

void AWindowJumpscareActor::TriggerFlickerEffect()
{
	if (!FlickerLight)
		return;

	FlickerLight->SetVisibility(true);
	FlickerLight->SetIntensity(FlickerIntensity * CurrentJumpscareIntensity);

	// Start flickering
	FlickerLightStep(0);
}

void AWindowJumpscareActor::FlickerLightStep(int32 CurrentFlicker)
{
	if (!FlickerLight || CurrentFlicker >= FlickerCount * 2)
	{
		// Done flickering
		if (FlickerLight)
		{
			FlickerLight->SetVisibility(false);
			FlickerLight->SetIntensity(0.0f);
		}
		return;
	}

	// Toggle light on/off
	bool bShouldBeOn = (CurrentFlicker % 2 == 0);
	FlickerLight->SetIntensity(bShouldBeOn ? FlickerIntensity * CurrentJumpscareIntensity : 0.0f);

	// Schedule next flicker
	GetWorldTimerManager().SetTimer(
		FlickerTimer,
		[this, CurrentFlicker]()
		{
			FlickerLightStep(CurrentFlicker + 1);
		},
		FlickerDuration,
		false
	);
}

void AWindowJumpscareActor::SelectRandomGhost()
{
	if (GhostMeshVariations.Num() > 0 && GhostMesh)
	{
		int32 RandomIndex = FMath::RandRange(0, GhostMeshVariations.Num() - 1);
		if (GhostMeshVariations[RandomIndex])
		{
			GhostMesh->SetStaticMesh(GhostMeshVariations[RandomIndex]);
		}
	}
}

USoundBase* AWindowJumpscareActor::GetRandomJumpscareSound()
{
	if (JumpscareSounds.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, JumpscareSounds.Num() - 1);
		return JumpscareSounds[RandomIndex];
	}

	return nullptr;
}

void AWindowJumpscareActor::CloseWindows()
{
	if (WindowTimeline)
	{
		WindowTimeline->Reverse();
	}

	if (GhostMesh)
	{
		GhostMesh->SetVisibility(false);
	}

	TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);

	if (PlayerCon && OriginalViewTarget)
	{
		PlayerCon->SetViewTargetWithBlend(OriginalViewTarget, 1.0f, EViewTargetBlendFunction::VTBlend_EaseInOut, 1.0f, false);
	}

	EnableInput(PlayerCon);

	// Make sure time scale is restored
	RestoreTimeScale();
}