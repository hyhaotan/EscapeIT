#include "GhostActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AGhostActor::AGhostActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create static mesh component (for simple ghost)
	GhostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GhostMesh"));
	RootComponent = GhostMesh;

	// Create skeletal mesh component (for animated ghost)
	GhostSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GhostSkeletalMesh"));
	GhostSkeletalMesh->SetupAttachment(RootComponent);
}

void AGhostActor::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
	FloatOffset = FMath::RandRange(0.0f, PI * 2.0f); // Random start phase

	CreateDynamicMaterials();
	UpdateMaterialOpacity(0.0f); // Start invisible

	// Schedule disappearance if set
	if (DisappearAfterTime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(DisappearTimerHandle, this, &AGhostActor::StartFadeOut, DisappearAfterTime, false);
	}

	UE_LOG(LogTemp, Warning, TEXT("👻 Ghost actor initialized"));
}

void AGhostActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsPaused)
		return;

	LifetimeTimer += DeltaTime;

	// Update fade in/out
	if (bIsFadingIn || bIsFadingOut)
	{
		UpdateFade(DeltaTime);
	}

	// Update rotation to face player
	if (bLookAtPlayer && !bIsFadingOut)
	{
		UpdateRotation(DeltaTime);
	}

	// Check if player is looking at ghost (for jumpscare)
	if (bEnableJumpscare && !bPlayerHasSeenGhost && !bIsFadingOut && CurrentFadeValue > 0.5f)
	{
		if (CheckGhostSeePlayer())
		{
			bPlayerHasSeenGhost = true;
			float JumpscareDelay = FMath::RandRange(JumpscareMinDelay, JumpscareMaxDelay);
			
			UE_LOG(LogTemp, Warning, TEXT("👻 Player spotted the ghost! Jumpscare in %.1f seconds..."), JumpscareDelay);
			
			GetWorldTimerManager().SetTimer(
				JumpscareTimerHandle,
				this,
				&AGhostActor::TriggerJumpscare,
				JumpscareDelay,
				false
			);
		}
	}
}

void AGhostActor::CreateDynamicMaterials()
{
	// Create dynamic materials for static mesh
	if (GhostMesh && GhostMesh->GetNumMaterials() > 0)
	{
		for (int32 i = 0; i < GhostMesh->GetNumMaterials(); i++)
		{
			UMaterialInstanceDynamic* DynMat = GhostMesh->CreateAndSetMaterialInstanceDynamic(i);
			if (DynMat)
			{
				DynamicMaterials.Add(DynMat);
			}
		}
	}

	// Create dynamic materials for skeletal mesh
	if (GhostSkeletalMesh && GhostSkeletalMesh->GetNumMaterials() > 0)
	{
		for (int32 i = 0; i < GhostSkeletalMesh->GetNumMaterials(); i++)
		{
			UMaterialInstanceDynamic* DynMat = GhostSkeletalMesh->CreateAndSetMaterialInstanceDynamic(i);
			if (DynMat)
			{
				DynamicMaterials.Add(DynMat);
			}
		}
	}
}

void AGhostActor::UpdateFade(float DeltaTime)
{
	if (bIsFadingIn)
	{
		FadeTimer += DeltaTime;
		CurrentFadeValue = FMath::Clamp(FadeTimer / FadeInDuration, 0.0f, 1.0f);

		// Ease-in curve for smoother fade
		float EasedValue = FMath::InterpEaseIn(0.0f, 1.0f, CurrentFadeValue, 2.0f);
		UpdateMaterialOpacity(EasedValue);

		if (CurrentFadeValue >= 1.0f)
		{
			bIsFadingIn = false;
			UE_LOG(LogTemp, Warning, TEXT("👻 Ghost fully materialized"));
		}
	}
	else if (bIsFadingOut)
	{
		FadeTimer -= DeltaTime;
		CurrentFadeValue = FMath::Clamp(FadeTimer / FadeInDuration, 0.0f, 1.0f);

		float EasedValue = FMath::InterpEaseOut(0.0f, 1.0f, CurrentFadeValue, 2.0f);
		UpdateMaterialOpacity(EasedValue);

		if (CurrentFadeValue <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("👻 Ghost disappeared"));
			Destroy();
		}
	}
}

void AGhostActor::UpdateRotation(float DeltaTime)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !PC->GetPawn())
		return;

	FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
	FVector GhostLocation = GetActorLocation();

	// Calculate rotation to face player
	FRotator LookAtRotation = (PlayerLocation - GhostLocation).Rotation();
	LookAtRotation.Pitch = 0.0f; // Keep ghost upright
	LookAtRotation.Roll = 0.0f;

	// Smooth rotation
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, 2.0f);

	SetActorRotation(NewRotation);
}

void AGhostActor::UpdateMaterialOpacity(float Opacity)
{
	for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
	{
		if (DynMat)
		{
			// Set opacity parameter
			DynMat->SetScalarParameterValue(OpacityParameterName, Opacity);

			// Optionally increase emissive strength as it fades in
			float EmissiveValue = Opacity * 2.0f;
			DynMat->SetScalarParameterValue(EmissiveStrengthParameterName, EmissiveValue);
		}
	}
}

void AGhostActor::StartFadeOut()
{
	bIsFadingOut = true;
	bIsFadingIn = false;
	FadeTimer = FadeInDuration * CurrentFadeValue; // Start from current opacity

	UE_LOG(LogTemp, Warning, TEXT("👻 Ghost starting to fade out..."));
}

bool AGhostActor::CheckGhostSeePlayer()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->GetPawn()) 
		return false;
	
	// Get player camera view
	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	
	// Get direction from camera to ghost (FIX: phải là trừ, không phải cộng)
	FVector ToGhost = GetActorLocation() - ViewLocation;
	float Distance = ToGhost.Size();
	ToGhost.Normalize();
	
	// Get camera forward vector
	FVector CameraForward = ViewRotation.Vector();
	
	// Calculate dot product (1.0 = looking directly at, 0.0 = perpendicular)
	float DotProduct = FVector::DotProduct(CameraForward, ToGhost);
	
	// Player is looking at ghost if dot product > threshold and within range
	bool bIsLookingAt = DotProduct > PlayerDetectionAngle && Distance < PlayerDetectionRange;
	
	// Optional: Line trace to check if ghost is visible (not behind walls)
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(PC->GetPawn());
	
	bool bHasLineOfSight = GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		ViewLocation, 
		GetActorLocation(), 
		ECC_Visibility, 
		CollisionParams
	);
	
	// If trace hit something, check if it's the ghost
	bool bCanSeeGhost = !bHasLineOfSight || (HitResult.GetActor() == this);
	
	return bIsLookingAt && bCanSeeGhost;
}

void AGhostActor::TriggerJumpscare()
{
	UE_LOG(LogTemp, Warning, TEXT("👻💀 JUMPSCARE! Ghost disappearing suddenly!"));

	// Call Blueprint event for additional effects (sound, camera shake, etc.)
	OnJumpscareTriggered();

	// Disappear immediately
	ForceDisappear();
	
	// Alternative: Use super fast fade out instead of instant disappear
	// FadeInDuration = 0.2f; // Very fast fade
	// StartFadeOut();
}

void AGhostActor::ForceDisappear()
{
	UE_LOG(LogTemp, Warning, TEXT("👻 Ghost force disappeared"));

	// Clear all timers
	GetWorldTimerManager().ClearTimer(DisappearTimerHandle);
	GetWorldTimerManager().ClearTimer(JumpscareTimerHandle);

	Destroy();
}

void AGhostActor::StartFadeOutNow()
{
	// Clear auto disappear timer and start fade out immediately
	GetWorldTimerManager().ClearTimer(DisappearTimerHandle);
	GetWorldTimerManager().ClearTimer(JumpscareTimerHandle);
	StartFadeOut();
}

void AGhostActor::PauseAllEffects()
{
	bIsPaused = true;
	UE_LOG(LogTemp, Warning, TEXT("👻 Ghost effects paused"));
}

void AGhostActor::ResumeAllEffects()
{
	bIsPaused = false;
	UE_LOG(LogTemp, Warning, TEXT("👻 Ghost effects resumed"));
}