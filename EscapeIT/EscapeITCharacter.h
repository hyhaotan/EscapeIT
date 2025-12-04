// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "EscapeITCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class USanityComponent;
class UInventoryComponent;
class UFlashlightComponent;
class UHeaderBobComponent;
class UFootstepComponent;
class UStaminaComponent;
class UAnimMontage;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

// ==================== ENUMS ====================

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Walking UMETA(DisplayName = "Walking"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Crouching UMETA(DisplayName = "Crouching")
};

// ==================== CHARACTER CLASS ====================

UCLASS(config=Game)
class AEscapeITCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	// ==================== COMPONENTS ====================

	/** First person mesh component (arms) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/** Sanity system component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USanityComponent> SanityComponent;

	/** Inventory system component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	/** Header bob (camera shake) component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeaderBobComponent> HeaderBobComponent;

	/** Footstep sound component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFootstepComponent> FootstepComponent;

	/** Stamina system component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaminaComponent> StaminaComponent;

	// ==================== INPUT ACTIONS ====================

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action (for gamepad) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseLookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CrouchAction;

public:
	AEscapeITCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;

	// ==================== INITIALIZATION ====================

	/** Initialize widget manager for UI */
	void InitializeWidgetManager();

	/** Initialize movement speeds */
	void InitializeMovementSpeeds();

	/** Bind component events */
	void BindComponentEvents();

	// ==================== INPUT HANDLERS ====================

	/** Called for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Detect input device (mouse or gamepad) */
	void DetectInputDevice();

	/** Handle aim input */
	void DoAim(float Yaw, float Pitch);

	/** Handle move input */
	void DoMove(float Right, float Forward);

	/** Handle jump start */
	void DoJumpStart();

	/** Handle jump end */
	void DoJumpEnd();

	// ==================== MOVEMENT ACTIONS ====================

	/** Start sprinting */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	/** Stop sprinting */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	/** Check if character can sprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement")
	bool CanSprint() const;
	virtual bool CanSprint_Implementation() const;

	// ==================== LANDING ====================

	/** Calculate landing impact strength */
	float CalculateLandingImpact(float FallSpeed) const;

	/** Play appropriate landing sound */
	void PlayLandingSound(float FallSpeed, float ImpactStrength);

	// ==================== STAMINA EVENTS ====================

	/** Called when stamina is exhausted */
	UFUNCTION()
	void OnStaminaExhausted();

	/** Called when stamina is recovered */
	UFUNCTION()
	void OnStaminaRecovered();

	// ==================== BLUEPRINT EVENTS ====================

	/** Blueprint event called when sprint starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnSprintStart();

	/** Blueprint event called when sprint stops */
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnSprintStop();

	/** Blueprint event called on landing impact */
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnLandingImpact(float ImpactStrength);

	/** Blueprint event called when stamina exhausted */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stamina")
	void BP_OnStaminaExhausted();

	/** Blueprint event called when stamina recovered */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stamina")
	void BP_OnStaminaRecovered();

public:
	// ==================== MOVEMENT PROPERTIES ====================

	/** Walking speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float WalkSpeed = 300.0f;

	/** Sprinting speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float SprintSpeed = 600.0f;

	/** Minimum speed required to start sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint")
	float MinSprintSpeed = 50.0f;

	// ==================== STAMINA PROPERTIES ====================

	/** Stamina cost for jumping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float JumpStaminaCost = 10.0f;

	/** Stamina cost for heavy landing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float HeavyLandingStaminaCost = 15.0f;

	// ==================== LANDING PROPERTIES ====================

	/** Light landing sound effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Landing")
	TObjectPtr<USoundBase> LightLandingSFX;

	/** Heavy landing sound effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Landing")
	TObjectPtr<USoundBase> HeavyLandingSFX;

	/** Landing sound volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Landing")
	float LandingSoundVolume = 1.0f;

	/** Light landing threshold (fall speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Landing")
	float LightLandingThreshold = 400.0f;

	/** Heavy landing threshold (fall speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Landing")
	float HeavyLandingThreshold = 600.0f;

	// ==================== STATE FLAGS ====================

	/** Is character currently sprinting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;
	
	// ==================== STATE DEATH ====================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Death")
	bool bIsDeath = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death")
	UAnimMontage* DeathMontage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsHoldingFlashlight;
	// ==================== GETTERS ====================

	/** Get current movement speed */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetCurrentSpeed() const;

	/** Get current speed as percentage of max speed */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetSpeedPercent() const;

	/** Get current movement state */
	UFUNCTION(BlueprintPure, Category = "Movement")
	EMovementState GetMovementState() const;

	/** Check if character is moving */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMoving() const;

	/** Check if character is sprinting */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const;

	// ==================== COMPONENT GETTERS ====================

	FORCEINLINE UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	FORCEINLINE USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	FORCEINLINE USanityComponent* GetSanityComponent() const { return SanityComponent; }
	FORCEINLINE UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE UHeaderBobComponent* GetHeaderBobComponent() const { return HeaderBobComponent; }
	FORCEINLINE UFootstepComponent* GetFootstepComponent() const { return FootstepComponent; }
	FORCEINLINE UStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }
	
private:
	//** Check if Sanity Character <= 0*/
	UFUNCTION(BlueprintPure, Category = "Death")
	bool IsDeath() const;
	
	UFUNCTION(Category = "Death")
	void CheckCharacterDeath();
	
};