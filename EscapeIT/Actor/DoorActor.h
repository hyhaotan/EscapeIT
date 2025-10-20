#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Interface/Interact.h"
#include "DoorActor.generated.h"

class UTimelineComponent;
class UStaticMeshComponent;
class USceneComponent;
class UCurveFloat;

UCLASS()
class ESCAPEIT_API ADoorActor : public AActor, public IInteract
{
	GENERATED_BODY()

public:
	ADoorActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Interface function từ IInteractableInterface
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	USceneComponent* DoorPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UStaticMeshComponent* DoorFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UStaticMeshComponent* Door;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UTimelineComponent* DoorTimeline;

	// Curve để điều khiển animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation")
	UCurveFloat* DoorCurve;

	// Góc mở cửa (độ)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings")
	float OpenAngle = 90.0f;

	// Trạng thái cửa
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
	bool bIsOpen;

	// Target rotation cho cửa
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
	FRotator DoorRotationTarget;

	// Speed của animation (tuỳ chỉnh)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation")
	float AnimationDuration = 1.0f;

private:
	// Hàm tính toán hướng mở cửa dựa vào vị trí player
	void CalculateDoorOpenDirection(AActor* Interactor);

	// Hàm callback từ Timeline để cập nhật rotation
	UFUNCTION()
	void UpdateDoorRotation(float Value);

	// Hàm mở cửa
	void OpenDoor();

	// Hàm đóng cửa
	void CloseDoor();

public:
	// Getter functions
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
	bool IsOpen() const { return bIsOpen; }
};