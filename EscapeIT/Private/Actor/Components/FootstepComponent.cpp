
#include "Actor/Components/FootstepComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"

UFootstepComponent::UFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;

	LastFootstepTime = 0.0f;
	CurrentFootstepInterval = WalkFootstepInterval;
}

void UFootstepComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("FootstepComponent must be attached to ACharacter!"));
	}
}

void UFootstepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableFootsteps || !bAutoPlayFootsteps)
	{
		return;
	}

	PlayFootstepSound();
}

void UFootstepComponent::PlayFootstepSound()
{
	if (!OwnerCharacter || !GetWorld())
	{
		return;
	}

	// Check if we should play footstep
	if (!ShouldPlayFootstep())
	{
		return;
	}

	// Update interval based on movement state
	UpdateFootstepInterval();

	// Check timing
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFootstepTime < CurrentFootstepInterval)
	{
		return;
	}

	LastFootstepTime = CurrentTime;

	// Trace for surface
	FHitResult HitResult;
	if (!TraceSurface(HitResult))
	{
		return; // No surface detected
	}

	// Get appropriate sound
	USoundBase* FootstepSound = GetFootstepSoundForSurface(HitResult);
	if (!FootstepSound)
	{
		return;
	}

	// Calculate volume
	float VolumeMultiplier = GetCurrentVolumeMultiplier();
	float FinalVolume = FootstepVolume * VolumeMultiplier;

	// Play sound
	UGameplayStatics::PlaySoundAtLocation(
		this,
		FootstepSound,
		OwnerCharacter->GetActorLocation(),
		FinalVolume
	);
}

bool UFootstepComponent::ShouldPlayFootstep() const
{
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();

	// Must be on ground
	if (!MovementComp->IsMovingOnGround())
	{
		return false;
	}

	// Must be moving
	float CurrentSpeed = OwnerCharacter->GetVelocity().Size2D();
	if (CurrentSpeed < MinimumSpeedThreshold)
	{
		return false;
	}

	return true;
}

void UFootstepComponent::UpdateFootstepInterval()
{
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	float CurrentSpeed = OwnerCharacter->GetVelocity().Size2D();

	// Determine interval based on movement state
	if (CurrentSpeed > 400.0f)
	{
		CurrentFootstepInterval = SprintFootstepInterval;
	}
	else // Walking
	{
		CurrentFootstepInterval = WalkFootstepInterval;
	}
}

float UFootstepComponent::GetCurrentVolumeMultiplier() const
{
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		return 1.0f;
	}

	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	float CurrentSpeed = OwnerCharacter->GetVelocity().Size2D();

	// Determine volume based on movement state
	if (CurrentSpeed > 400.0f)
	{
		return SprintVolumeMultiplier;
	}
	else // Walk
	{
		return WalkVolumeMultiplier;
	}
}

bool UFootstepComponent::TraceSurface(FHitResult& OutHitResult) const
{
	if (!OwnerCharacter || !GetWorld())
	{
		return false;
	}

	FVector StartLocation = OwnerCharacter->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0, 0, TraceDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// Debug visualization
	if (bDebugTrace)
	{
		DrawDebugLine(
			GetWorld(),
			StartLocation,
			EndLocation,
			bHit ? FColor::Green : FColor::Red,
			false,
			0.1f,
			0,
			1.0f
		);

		if (bHit)
		{
			DrawDebugSphere(
				GetWorld(),
				OutHitResult.ImpactPoint,
				10.0f,
				12,
				FColor::Green,
				false,
				0.1f
			);
		}
	}

	return bHit;
}

USoundBase* UFootstepComponent::GetFootstepSoundForSurface(const FHitResult& HitResult) const
{
	// Try to get physical material from hit result
	if (HitResult.PhysMaterial.IsValid())
	{
		UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get();
		if (PhysMat)
		{
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMat);

			// Map surface types to sounds
			switch (SurfaceType)
			{
			case SurfaceType1: // Wood
				return FootstepWoodSFX ? FootstepWoodSFX : FootstepDefaultSFX;

			case SurfaceType2: // Metal
				return FootstepMetalSFX ? FootstepMetalSFX : FootstepDefaultSFX;

			case SurfaceType3: // Concrete
				return FootstepConcreteSFX ? FootstepConcreteSFX : FootstepDefaultSFX;

			case SurfaceType4: // Grass
				return FootstepGrassSFX ? FootstepGrassSFX : FootstepDefaultSFX;

			case SurfaceType5: // Water
				return FootstepWaterSFX ? FootstepWaterSFX : FootstepDefaultSFX;

			case SurfaceType6: // Carpet
				return FootstepCarpetSFX ? FootstepCarpetSFX : FootstepDefaultSFX;

			default:
				return FootstepDefaultSFX;
			}
		}
	}

	// Fallback to default
	return FootstepDefaultSFX;
}