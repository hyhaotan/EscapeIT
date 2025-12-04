// Flashlight.h
#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Flashlight.generated.h"

class USpotLightComponent;
class UFlashlightComponent;

UCLASS()
class ESCAPEIT_API AFlashlight : public AItemPickupActor
{
	GENERATED_BODY()

public:
	AFlashlight();

	virtual void BeginPlay() override;

	// ============================================
	// COMPONENTS
	// ============================================
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpotLightComponent* SpotLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFlashlightComponent* FlashlightComponent;

	// ============================================
	// EVENT HANDLERS
	// ============================================
    
	UFUNCTION()
	void OnFlashlightToggled(bool bIsOn);

	UFUNCTION()
	void OnBatteryChanged(float Current, float Max);

	UFUNCTION()
	void OnBatteryLow();

	UFUNCTION()
	void OnBatteryDepleted();
};