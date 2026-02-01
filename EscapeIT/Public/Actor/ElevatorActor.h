#pragma once

#include "CoreMinimal.h"
#include "Actor/InteractableActor.h"
#include "Components/BoxComponent.h"
#include "ElevatorActor.generated.h"

class UAnimSequenceBase;

UENUM(BlueprintType)
enum class EFloorType : uint8
{
    GroundFloor UMETA(DisplayName = "Ground Floor"),
    Floor_1 UMETA(DisplayName = "Floor 1"),
    Floor_2 UMETA(DisplayName = "Floor 2"),
    Floor_3 UMETA(DisplayName = "Floor 3"),
    Floor_4 UMETA(DisplayName = "Floor 4"),
    Floor_5 UMETA(DisplayName = "Floor 5")
};

UENUM(BlueprintType)
enum class EElevatorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    OpeningDoor UMETA(DisplayName = "Opening Door"),
    WaitingForPassenger UMETA(DisplayName = "Waiting"),
    ClosingDoor UMETA(DisplayName = "Closing Door"),
    Moving UMETA(DisplayName = "Moving"),
    Arrived UMETA(DisplayName = "Arrived")
};

UCLASS()
class ESCAPEIT_API AElevatorActor : public AInteractableActor
{
    GENERATED_BODY()

public:
    AElevatorActor();
    virtual void Tick(float DeltaTime) override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    
    UFUNCTION(BlueprintCallable, Category = "Elevator")
    void RequestFloor(int32 FloorNumber);
    
    UFUNCTION(BlueprintCallable, Category = "Elevator")
    bool IsPlayerInside() const { return bPlayerInside; }
    
    EFloorType GetCurrentFloor() const { return CurrentFloor; }
    EElevatorState GetElevatorState() const { return ElevatorState; }

protected:
    virtual void BeginPlay() override;
    
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
    TObjectPtr<USceneComponent> ElevatorRoot;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
    TObjectPtr<USkeletalMeshComponent> ElevatorMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
    TObjectPtr<UBoxComponent> InteractionBox;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator")
    TObjectPtr<UBoxComponent> PlayerDetectionBox;
    
    // Animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elevator|Animation")
    TObjectPtr<UAnimSequenceBase> DoorOpenAnim;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elevator|Animation")
    TObjectPtr<UAnimSequenceBase> DoorCloseAnim;
    
    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elevator|Settings")
    float MoveSpeed = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elevator|Settings")
    float DoorOpenTime = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elevator|Settings")
    float DoorCloseTime = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elevator|Settings")
    float WaitTimeAfterDoorOpen = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elevator|Settings")
    TMap<EFloorType, float> FloorHeights;
    
    virtual void OnInteractionBeginOverlap_Implementation(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult) override;
    
    virtual void OnInteractionEndOverlap_Implementation(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex) override;

private:
    // State
    EFloorType CurrentFloor = EFloorType::GroundFloor;
    EFloorType TargetFloor = EFloorType::GroundFloor;
    EElevatorState ElevatorState = EElevatorState::Idle;
    
    bool bPlayerInside = false;
    bool bDoorOpen = false;
    float StateTimer = 0.0f;
    FText InteractionPrompt;
    
    FVector StartPosition;
    FVector TargetPosition;
    float MoveProgress = 0.0f;
    
    // Functions
    void UpdateElevatorState(float DeltaTime);
    void OpenDoor();
    void CloseDoor();
    void PlayDoorAnimation(UAnimSequenceBase* Animation);
    float GetFloorHeight(EFloorType Floor) const;
};