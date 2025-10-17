#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Interface/Interact.h"
#include "DoorActor.generated.h"

class UStaticMeshComponent;
class UTimelineComponent;
class UCurveFloat;

UCLASS()
class ESCAPEIT_API ADoorActor : public AActor, public IInteract
{
	GENERATED_BODY()

public:
	ADoorActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	// ======================================== //
	// ============= PROPERTIES ============= //
	// ======================================== //
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<UStaticMeshComponent> DoorFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Door;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<UTimelineComponent> DoorTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<UCurveFloat> DoorCurve;

private:
	// ======================================== //
	// ============= PROPERTIES ============= //
	// ======================================== //
	bool bIsOpen;

	// Rotator cuối cùng khi cửa mở hoàn toàn (tính bằng độ)
	FRotator DoorRot{ 0.0f, 90.0f, 0.0f };

	// ======================================== //
	// ============= FUNCTIONS ============= //
	// ======================================== //
	void OpenDoor();
	void CloseDoor();

	// Callback từ Timeline khi update
	UFUNCTION()
	void UpdateDoorRotation(float Value);
};