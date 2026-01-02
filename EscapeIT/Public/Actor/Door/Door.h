#pragma once

#include "CoreMinimal.h"
#include "Actor/InteractableActor.h"
#include "Door.generated.h"

class UTimelineComponent;
class UStaticMeshComponent;
class USceneComponent;
class UCurveFloat;
class UAudioManager;
class UBoxComponent;

UCLASS()
class ESCAPEIT_API ADoor : public AInteractableActor 
{
    GENERATED_BODY()

public:
    ADoor();

public:
    // ========================= COMPONENTS ===========================
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    TObjectPtr<USceneComponent> DoorPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    TObjectPtr<UStaticMeshComponent> DoorFrame;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    TObjectPtr<UStaticMeshComponent> Door;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    TObjectPtr<UTimelineComponent> DoorTimeline;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    TObjectPtr<UBoxComponent> InteractBox;
    
    // =========================== PROPERTIES ==========================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation")
    TObjectPtr<UCurveFloat> DoorCurve;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenAngle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    bool bIsOpen;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    FRotator DoorRotationTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float AnimationDuration;
    
    // ========================= DOOR FUNCTIONS ===========================
    
    UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Door")
    void UpdateDoorRotation(float Value);
    
    UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Door")
    void OpenDoor();

    UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Door")
    void CloseDoor();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    bool IsOpen() const { return bIsOpen; }

    // ========================= OVERRIDE  ===========================
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
    virtual void BeginPlay() override;
    
    virtual void Tick(float DeltaTime) override;

protected:
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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioManager> AudioManager;
};