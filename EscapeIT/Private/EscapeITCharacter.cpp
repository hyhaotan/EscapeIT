// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeITCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EscapeIT.h"
#include "Actor/Components/SanityComponent.h"
#include "Actor/Components/InventoryComponent.h"
#include "Actor/Components/HeaderBobComponent.h"
#include "Actor/Components/FootstepComponent.h"
#include "Actor/Components/StaminaComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/WidgetManager.h"
#include "EscapeITPlayerController.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"
#include "Actor/Components/FlashlightComponent.h"
#include "Perception/AISense_Hearing.h"

// ==================== CONSTRUCTOR ====================

AEscapeITCharacter::AEscapeITCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// ========== FIRST PERSON MESH SETUP ==========
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Enable shadow for body awareness
	FirstPersonMesh->bCastHiddenShadow = true;
	FirstPersonMesh->bCastDynamicShadow = true;
	FirstPersonMesh->SetForcedLOD(1);
	FirstPersonMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// ========== CAMERA SETUP ==========
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(
		FVector(-2.8f, 5.89f, 0.0f), 
		FRotator(0.0f, 90.0f, -90.0f)
	);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// ========== THIRD PERSON MESH CONFIG ==========
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// ========== CHARACTER MOVEMENT CONFIG ==========
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// ========== CREATE COMPONENTS ==========
	SanityComponent = CreateDefaultSubobject<USanityComponent>(TEXT("SanityComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	HeaderBobComponent = CreateDefaultSubobject<UHeaderBobComponent>(TEXT("HeaderBobComponent"));
	FootstepComponent = CreateDefaultSubobject<UFootstepComponent>(TEXT("FootstepComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	FlashlightComponent = CreateDefaultSubobject<UFlashlightComponent>(TEXT("FlashlightComponent"));

	// Enable tick
	PrimaryActorTick.bCanEverTick = true;
	
	AnimState = EAnimState::Unarmed;
}

// ==================== INITIALIZATION ====================

void AEscapeITCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize Widget Manager for UI
	InitializeWidgetManager();

	// Initialize movement speeds
	InitializeMovementSpeeds();

	// Bind component events
	BindComponentEvents();
	
	SetGenericTeamId(FGenericTeamId(1));
}

void AEscapeITCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetVelocity().Size() > 0.0f)
	{
		TimeSinceLastFootstep += DeltaTime;

		if (TimeSinceLastFootstep >= FootstepNoiseInterval)
		{
			MakeFootstepNoise();
			TimeSinceLastFootstep = 0.0f;
		}
	}
	else
	{
		TimeSinceLastFootstep = 0.0f;
	}
	
	if (bIsSprinting && StaminaComponent)
	{
		if (StaminaComponent->GetCurrentStamina() <= 0.0f || 
			StaminaComponent->IsExhausted())
		{
			StopSprint();
		}
	}
	
	CheckCharacterDeath();
}

void AEscapeITCharacter::InitializeWidgetManager()
{
	const auto PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	const auto WidgetMgr = Cast<AWidgetManager>(PC->GetHUD());
	if (!WidgetMgr)
	{
		return;
	}

	// Initialize Sanity UI
	if (SanityComponent)
	{
		WidgetMgr->InitializeSanityWidget(SanityComponent);
	}
}

void AEscapeITCharacter::InitializeMovementSpeeds()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AEscapeITCharacter::BindComponentEvents()
{
	// Bind stamina events
	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaExhausted.AddDynamic(this, &AEscapeITCharacter::OnStaminaExhausted);
		StaminaComponent->OnStaminaRecovered.AddDynamic(this, &AEscapeITCharacter::OnStaminaRecovered);
	}
	
	if (FlashlightComponent)
	{
		// Bind to flashlight events
		FlashlightComponent->OnFlashlightToggled.AddDynamic(this, &AEscapeITCharacter::OnFlashlightToggled);
		FlashlightComponent->OnBatteryChanged.AddDynamic(this, &AEscapeITCharacter::OnBatteryChanged);
		FlashlightComponent->OnBatteryLow.AddDynamic(this, &AEscapeITCharacter::OnBatteryLow);
		FlashlightComponent->OnBatteryDepleted.AddDynamic(this, &AEscapeITCharacter::OnBatteryDepleted);
        
		UE_LOG(LogTemp, Log, TEXT("AFlashlight: FlashlightComponent initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AFlashlight: FlashlightComponent is NULL!"));
	}
}

// ==================== INPUT SETUP ====================

void AEscapeITCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AEscapeITCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AEscapeITCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEscapeITCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEscapeITCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AEscapeITCharacter::LookInput);

		// Sprint
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AEscapeITCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AEscapeITCharacter::StopSprint);
		}
	}
	else
	{
		UE_LOG(LogEscapeIT, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

// ==================== INPUT HANDLERS ====================

void AEscapeITCharacter::MoveInput(const FInputActionValue& Value)
{
	// Get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// Pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AEscapeITCharacter::LookInput(const FInputActionValue& Value)
{
	// Get axis 2D from Enhanced Input
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// Try to detect input device source and notify PlayerController
	DetectInputDevice();

	// Pass rotation to aim system
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AEscapeITCharacter::DetectInputDevice()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Check mouse delta first (mouse has priority)
		float MouseDX = 0.f;
		float MouseDY = 0.f;
		PC->GetInputMouseDelta(MouseDX, MouseDY);

		if (!FMath::IsNearlyZero(MouseDX) || !FMath::IsNearlyZero(MouseDY))
		{
			// Mouse input detected
			if (AEscapeITPlayerController* EPC = Cast<AEscapeITPlayerController>(PC))
			{
				EPC->NotifyMouseInput();
			}
		}
		else
		{
			// Check gamepad analog state (right stick)
			const float GamepadX = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX);
			const float GamepadY = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightY);

			if (!FMath::IsNearlyZero(GamepadX) || !FMath::IsNearlyZero(GamepadY))
			{
				// Gamepad input detected
				if (AEscapeITPlayerController* EPC = Cast<AEscapeITPlayerController>(PC))
				{
					EPC->NotifyGamepadInput();
				}
			}
		}
	}
}

void AEscapeITCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AEscapeITCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

// ==================== JUMP ====================

void AEscapeITCharacter::DoJumpStart()
{
	// Cannot jump while crouching
	if (bIsCrouched)
	{
		return;
	}

	// Drain stamina when jumping
	if (StaminaComponent)
	{
		StaminaComponent->DrainStamina(JumpStaminaCost);
	}

	if (GetCharacterMovement()->IsFalling())
	{
		MakeNoise(JumpNoiseLoudness,GetActorLocation());
	}

	Jump();
}

void AEscapeITCharacter::DoJumpEnd()
{
	StopJumping();
}

void AEscapeITCharacter::MakeNoise(float Loudness, FVector NoiseLocation)
{
	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		NoiseLocation,
		Loudness,
		this,
		0.0f,
		FName("PlayerNoise"));
}

// ==================== SPRINT ====================

void AEscapeITCharacter::StartSprint()
{
	// Cannot sprint while crouching
	if (bIsCrouched)
	{
		return;
	}

	// Check stamina first
	if (StaminaComponent && !StaminaComponent->CanSprint())
	{
		return;
	}

	// Check other sprint conditions
	if (!CanSprint())
	{
		return;
	}

	if (GetCharacterMovement())
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

		// Start draining stamina
		if (StaminaComponent)
		{
			StaminaComponent->StartDraining();
		}

		// Blueprint event
		OnSprintStart();

		UE_LOG(LogTemp, Log, TEXT("Sprint Started - Speed: %.2f"), SprintSpeed);
	}
}

void AEscapeITCharacter::StopSprint()
{
	if (!bIsSprinting)
	{
		return;
	}

	if (GetCharacterMovement())
	{
		bIsSprinting = false;
		
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

		// Stop draining stamina
		if (StaminaComponent)
		{
			StaminaComponent->StopDraining();
		}

		// Blueprint event
		OnSprintStop();

		UE_LOG(LogTemp, Log, TEXT("Sprint Stopped - Speed: %.2f"), GetCharacterMovement()->MaxWalkSpeed);
	}
}

bool AEscapeITCharacter::CanSprint_Implementation() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	// Cannot sprint while crouched
	if (bIsCrouched)
	{
		return false;
	}

	// Must be on ground
	if (!GetCharacterMovement()->IsMovingOnGround())
	{
		return false;
	}

	// Must be moving forward
	float Speed = GetVelocity().Size2D();
	if (Speed < MinSprintSpeed)
	{
		return false;
	}

	// Check if moving forward (not backward)
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	Velocity.Normalize();

	FVector Forward = GetActorForwardVector();
	float DotProduct = FVector::DotProduct(Velocity, Forward);

	// Only sprint if moving mostly forward
	return DotProduct > 0.5f;
}

// ==================== STAMINA EVENTS ====================

void AEscapeITCharacter::OnStaminaExhausted()
{
	// Auto-stop sprint when exhausted
	if (bIsSprinting)
	{
		StopSprint();
	}

	// Blueprint event
	BP_OnStaminaExhausted();
}

void AEscapeITCharacter::OnStaminaRecovered()
{
	// Blueprint event
	BP_OnStaminaRecovered();
}

// ==================== LANDING ====================

void AEscapeITCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Get fall speed for impact calculation
	float FallSpeed = FMath::Abs(GetVelocity().Z);

	// Trigger landing impact in HeaderBobComponent
	if (HeaderBobComponent)
	{
		HeaderBobComponent->TriggerLandingImpact(FallSpeed);
	}

	// Calculate impact strength for blueprint event
	float ImpactStrength = CalculateLandingImpact(FallSpeed);
	OnLandingImpact(ImpactStrength);

	// Play landing sounds
	PlayLandingSound(FallSpeed, ImpactStrength);

	// Stop sprint on landing
	if (bIsSprinting)
	{
		StopSprint();
	}

	// Drain stamina based on fall height
	if (StaminaComponent && FallSpeed > HeavyLandingThreshold)
	{
		float StaminaDrain = FMath::Lerp(0.0f, HeavyLandingStaminaCost, (FallSpeed - HeavyLandingThreshold) / 600.0f);
		StaminaComponent->DrainStamina(StaminaDrain);
	}
}

float AEscapeITCharacter::CalculateLandingImpact(float FallSpeed) const
{
	return FMath::Clamp((FallSpeed - LightLandingThreshold) / (HeavyLandingThreshold - LightLandingThreshold), 0.0f, 1.0f);
}

void AEscapeITCharacter::PlayLandingSound(float FallSpeed, float ImpactStrength)
{
	if (FallSpeed > HeavyLandingThreshold && HeavyLandingSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, 
			HeavyLandingSFX, 
			GetActorLocation(),
			FMath::Clamp(ImpactStrength, 0.5f, 1.0f) * LandingSoundVolume
		);
	}
	else if (FallSpeed > LightLandingThreshold && LightLandingSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, 
			LightLandingSFX, 
			GetActorLocation(),
			FMath::Clamp(ImpactStrength, 0.3f, 0.7f) * LandingSoundVolume
		);
	}
}

void AEscapeITCharacter::MakeFootstepNoise()
{
	float Speed = GetVelocity().Size();

	if (Speed > 200.0f)
	{
		float Loudness = FootstepNoiseLoudness;

		if (Speed > 400.0f)
		{
			Loudness *= 1.5f;
		}
		
		MakeNoise(Loudness,GetActorLocation());
	}
}

// ==================== GETTERS ====================

float AEscapeITCharacter::GetCurrentSpeed() const
{
	if (GetCharacterMovement())
	{
		return GetVelocity().Size2D();
	}
	return 0.0f;
}

float AEscapeITCharacter::GetSpeedPercent() const
{
	if (GetCharacterMovement())
	{
		float MaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
		if (MaxSpeed > 0.0f)
		{
			return GetCurrentSpeed() / MaxSpeed;
		}
	}
	return 0.0f;
}

EMovementState AEscapeITCharacter::GetMovementState() const
{
	if (bIsCrouched)
	{
		return EMovementState::Crouching;
	}
	
	if (bIsSprinting)
	{
		return EMovementState::Sprinting;
	}

	float Speed = GetCurrentSpeed();
	if (Speed < 10.0f)
	{
		return EMovementState::Idle;
	}

	return EMovementState::Walking;
}

bool AEscapeITCharacter::IsMoving() const
{
	return GetCurrentSpeed() > 10.0f;
}

bool AEscapeITCharacter::IsSprinting() const
{
	return bIsSprinting;
}

bool AEscapeITCharacter::IsDeath() const
{
	return bIsDeath;
}

void AEscapeITCharacter::CheckCharacterDeath()
{
	if (bIsDeath)
	{
		return;
	}

	if (SanityComponent && SanityComponent->GetSanity() <= 0)
	{
		bIsDeath = true;

		if (DeathMontage)
		{
			PlayAnimMontage(DeathMontage);
		}
		
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle,[this]()
		{
			if (const auto PC = UGameplayStatics::GetPlayerController(this,0))
			{
				const auto WidgetMgr = Cast<AWidgetManager>(PC->GetHUD());
				if (WidgetMgr && WidgetMgr->GetDeathWidget())
				{
					WidgetMgr->ShowWidget(WidgetMgr->GetDeathWidget());
				
					PC->bShowMouseCursor = true;
					PC->SetInputMode(FInputModeUIOnly());
				}
				DisableInput(PC);
			}
		},3.0f,false);
	}
}

void AEscapeITCharacter::OnFlashlightToggled(bool bIsOn)
{
	UE_LOG(LogTemp, Log, TEXT("Flashlight toggled: %s"), bIsOn ? TEXT("ON") : TEXT("OFF"));
}

void AEscapeITCharacter::OnBatteryChanged(float Current, float Max)
{
	float Percentage = (Current / Max) * 100.0f;
	UE_LOG(LogTemp, Log, TEXT("Battery: %.1f%%"), Percentage);
}

void AEscapeITCharacter::OnBatteryLow()
{
	UE_LOG(LogTemp, Warning, TEXT("LOW BATTERY WARNING!"));
}

void AEscapeITCharacter::OnBatteryDepleted()
{
	UE_LOG(LogTemp, Error, TEXT("BATTERY DEPLETED!"));
}