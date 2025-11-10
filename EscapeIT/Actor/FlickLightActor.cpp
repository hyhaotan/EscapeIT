// Fill out your copyright notice in the Description page of Project Settings.

#include "FlickLightActor.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "EscapeIT/Actor/GhostActor.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"

AFlickLightActor::AFlickLightActor()
{
	PrimaryActorTick.bCanEverTick = true;

	LightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightMesh"));
	RootComponent = LightMesh;

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->SetupAttachment(LightMesh);
	PointLight->SetVisibility(true);
	PointLight->SetIntensity(NormalLightIntensity);

	GhostSpawnLocation = FVector(0.0f, 0.0f, -300.0f);
	GhostSpawnRotation = FRotator(0.0f, 180.0f, 0.0f);
}

void AFlickLightActor::BeginPlay()
{
	Super::BeginPlay();

	// Set initial light properties
	PointLight->SetIntensity(NormalLightIntensity);
	PointLight->SetLightColor(NormalLightColor);

	// Start ambient drone sound
	if (AmbientDroneSound)
	{
		AmbientDroneAudioComponent = UGameplayStatics::SpawnSoundAttached(
			AmbientDroneSound,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			false,
			AmbientDroneVolume,
			1.0f
		);
	}

	// Auto start flicker sequence
	GetWorldTimerManager().SetTimer(DelayTimerHandle, this, &AFlickLightActor::StartFlickerSequence, DelayBeforeFlicker, false);
}

void AFlickLightActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInDramaticPause)
	{
		UpdateDramaticPause(DeltaTime);
	}
	else if (bIsFlickering)
	{
		UpdateFlicker(DeltaTime);
	}
}

void AFlickLightActor::StartFlickerSequence()
{
	if (bSequenceStarted)
		return;

	bSequenceStarted = true;
	bIsFlickering = true;
	bHasSpawnedGhost = false;
	bInDramaticPause = false;
	FlickerTimer = 0.0f;
	TotalFlickerTime = 0.0f;
	FlickerCount = 0;
	NextFlickerTime = GetCurrentFlickerInterval();
	bIsLightOn = true;

	// Play flicker start sound
	if (FlickerStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FlickerStartSound, GetActorLocation());
	}

	// Increase ambient drone volume
	if (AmbientDroneAudioComponent)
	{
		AmbientDroneAudioComponent->SetVolumeMultiplier(AmbientDroneVolume * 2.0f);
	}

	UE_LOG(LogTemp, Warning, TEXT("🔴 Flicker sequence started - Horror mode activated!"));
}

void AFlickLightActor::UpdateFlicker(float DeltaTime)
{
	FlickerTimer += DeltaTime;
	TotalFlickerTime += DeltaTime;

	// Check if we should toggle the light
	if (FlickerTimer >= NextFlickerTime)
	{
		ToggleLight();
		FlickerTimer = 0.0f;
		FlickerCount++;
		NextFlickerTime = GetCurrentFlickerInterval();

		// Play flicker sound
		PlayFlickerSound();

		// Spawn electrical sparks occasionally
		if (ElectricalSparkParticle && FMath::RandRange(0.0f, 1.0f) > 0.7f)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElectricalSparkParticle, GetActorLocation());
		}

		// Trigger camera shake
		if (LightFlickerCameraShake && FMath::RandRange(0.0f, 1.0f) > 0.6f)
		{
			TriggerCameraShake(LightFlickerCameraShake);
		}
	}

	// Start dramatic pause before ghost spawn
	if (bEnableDramaticPause && !bHasSpawnedGhost && FlickerCount >= NumberOfFlickersBeforeGhost)
	{
		StartDramaticPause();
	}

	// Stop flickering after duration (if no dramatic pause)
	if (!bEnableDramaticPause && TotalFlickerTime >= FlickerDuration)
	{
		if (!bHasSpawnedGhost)
		{
			SpawnGhost();
		}
		StopFlicker();
	}
}

void AFlickLightActor::StartDramaticPause()
{
	bInDramaticPause = true;
	bIsFlickering = false;
	DramaticPauseTimer = 0.0f;

	// Turn off light completely
	PointLight->SetVisibility(false);
	bIsLightOn = false;

	// Increase tension with audio
	if (AmbientDroneAudioComponent)
	{
		AmbientDroneAudioComponent->SetVolumeMultiplier(AmbientDroneVolume * 3.0f);
		AmbientDroneAudioComponent->SetPitchMultiplier(0.8f); // Lower pitch for more dread
	}

	UE_LOG(LogTemp, Warning, TEXT("😱 Dramatic pause - Something is coming..."));
}

void AFlickLightActor::UpdateDramaticPause(float DeltaTime)
{
	DramaticPauseTimer += DeltaTime;

	if (DramaticPauseTimer >= DramaticPauseDuration)
	{
		// Spawn ghost during darkness
		if (!bHasSpawnedGhost)
		{
			SpawnGhost();
		}

		// Turn light back on after short delay
		FTimerHandle LightOnTimer;
		GetWorldTimerManager().SetTimer(LightOnTimer, [this]()
			{
				StopFlicker();
			}, 0.5f, false);

		bInDramaticPause = false;
	}
}

void AFlickLightActor::ToggleLight()
{
	bIsLightOn = !bIsLightOn;
	PointLight->SetVisibility(bIsLightOn);

	if (bIsLightOn)
	{
		// Random intensity variation for more realistic flicker
		float IntensityVariation = FMath::RandRange(0.7f, 1.3f);
		PointLight->SetIntensity(FlickerLightIntensity * IntensityVariation);

		if (bEnableLightColorChange)
		{
			// Shift towards red/orange during flicker
			FLinearColor FlickerColor = FMath::Lerp(NormalLightColor, FlickerLightColor,
				FMath::RandRange(0.3f, 0.8f));
			PointLight->SetLightColor(FlickerColor);
		}
	}
}

void AFlickLightActor::SpawnGhost()
{
	if (!GhostActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ GhostActorClass is not set!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
		return;

	FVector LightLoc = LightMesh->GetRelativeLocation();
	FVector SpawnLocation = LightLoc + GhostSpawnLocation;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGhostActor* Ghost = World->SpawnActor<AGhostActor>(GhostActorClass, SpawnLocation, GhostSpawnRotation, SpawnParams);

	if (Ghost)
	{
		bHasSpawnedGhost = true;

		// Thêm ghost vào danh sách để quản lý
		SpawnedGhosts.Add(Ghost);

		// Play ghost appear sound
		if (GhostAppearSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, GhostAppearSound, SpawnLocation, 1.5f);
		}

		// Spawn ghost appearance particle effect
		if (GhostAppearParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, GhostAppearParticle, SpawnLocation);
		}

		// Trigger strong camera shake
		if (GhostAppearCameraShake)
		{
			TriggerCameraShake(GhostAppearCameraShake);
		}

		UE_LOG(LogTemp, Warning, TEXT("👻 GHOST SPAWNED at location: %s"), *SpawnLocation.ToString());
	}
}

void AFlickLightActor::StopFlicker()
{
	bIsFlickering = false;
	bInDramaticPause = false;

	// Restore normal light
	PointLight->SetVisibility(true);
	PointLight->SetIntensity(NormalLightIntensity);
	PointLight->SetLightColor(NormalLightColor);
	bIsLightOn = true;

	// Fade out ambient drone
	if (AmbientDroneAudioComponent)
	{
		AmbientDroneAudioComponent->FadeOut(2.0f, 0.0f);
	}

	// Handle auto reset or loop
	if (bAutoResetAfterComplete || bLoopSequence)
	{
		GetWorldTimerManager().SetTimer(AutoResetTimerHandle, this, &AFlickLightActor::HandleAutoReset, AutoResetDelay, false);
	}

	UE_LOG(LogTemp, Warning, TEXT("✅ Flicker sequence completed!"));
}

void AFlickLightActor::StopSequenceImmediately()
{
	bSequenceStarted = false;
	StopFlicker();

	// Clear all timers
	GetWorldTimerManager().ClearTimer(AutoResetTimerHandle);
	GetWorldTimerManager().ClearTimer(DelayTimerHandle);

	if (AmbientDroneAudioComponent)
	{
		AmbientDroneAudioComponent->Stop();
	}

	// Destroy all spawned ghosts
	DestroySpawnedGhosts();
}

void AFlickLightActor::ResetSequence()
{
	UE_LOG(LogTemp, Warning, TEXT("🔄 Resetting flicker sequence..."));

	// Stop everything
	StopSequenceImmediately();

	// Reset all variables
	ResetAllVariables();

	UE_LOG(LogTemp, Warning, TEXT("✅ Sequence reset complete - Ready to start again"));
}

void AFlickLightActor::RestartSequence()
{
	UE_LOG(LogTemp, Warning, TEXT("🔁 Restarting flicker sequence..."));

	// Reset first
	ResetSequence();

	// Start again with delay
	GetWorldTimerManager().SetTimer(DelayTimerHandle, this, &AFlickLightActor::StartFlickerSequence, DelayBeforeFlicker, false);
}

void AFlickLightActor::ResetAllVariables()
{
	bIsFlickering = false;
	bSequenceStarted = false;
	bInDramaticPause = false;
	bHasSpawnedGhost = false;
	bIsLightOn = true;

	FlickerTimer = 0.0f;
	NextFlickerTime = 0.0f;
	TotalFlickerTime = 0.0f;
	DramaticPauseTimer = 0.0f;
	FlickerCount = 0;

	// Restore light to normal state
	if (PointLight)
	{
		PointLight->SetVisibility(true);
		PointLight->SetIntensity(NormalLightIntensity);
		PointLight->SetLightColor(NormalLightColor);
	}

	// Reset ambient drone
	if (AmbientDroneAudioComponent && AmbientDroneSound)
	{
		AmbientDroneAudioComponent->Stop();
		AmbientDroneAudioComponent = UGameplayStatics::SpawnSoundAttached(
			AmbientDroneSound,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			false,
			AmbientDroneVolume,
			1.0f
		);
	}
}

void AFlickLightActor::DestroySpawnedGhosts()
{
	for (AActor* Ghost : SpawnedGhosts)
	{
		if (Ghost)
		{
			Ghost->Destroy();
		}
	}
	SpawnedGhosts.Empty();

	UE_LOG(LogTemp, Warning, TEXT("👻 All spawned ghosts destroyed"));
}

void AFlickLightActor::HandleAutoReset()
{
	if (bLoopSequence)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔁 Loop enabled - Restarting sequence..."));
		RestartSequence();
	}
	else if (bAutoResetAfterComplete)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔄 Auto reset - Resetting to initial state..."));
		ResetSequence();
	}
}

void AFlickLightActor::PlayFlickerSound()
{
	if (FlickerSound && FMath::RandRange(0.0f, 1.0f) > 0.5f)
	{
		float PitchVariation = FMath::RandRange(0.9f, 1.1f);
		float VolumeVariation = FMath::RandRange(0.3f, 0.7f);
		UGameplayStatics::PlaySoundAtLocation(this, FlickerSound, GetActorLocation(),
			VolumeVariation, PitchVariation);
	}
}

void AFlickLightActor::TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass)
{
	if (!ShakeClass)
		return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->ClientStartCameraShake(ShakeClass, CameraShakeScale);
	}
}

float AFlickLightActor::GetCurrentFlickerInterval()
{
	if (bIntensifyFlickerOverTime)
	{
		// Flicker gets faster over time for increased tension
		float Progress = TotalFlickerTime / FlickerDuration;
		float CurrentMin = FMath::Lerp(MinFlickerInterval, MinFlickerInterval * 0.5f, Progress);
		float CurrentMax = FMath::Lerp(MaxFlickerInterval, MaxFlickerInterval * 0.6f, Progress);
		return FMath::RandRange(CurrentMin, CurrentMax);
	}

	return FMath::RandRange(MinFlickerInterval, MaxFlickerInterval);
}