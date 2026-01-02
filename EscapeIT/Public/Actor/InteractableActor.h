#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interact.h"
#include "InteractableActor.generated.h"

class UWidgetComponent;
class UInteractionPromptWidget;
class UPrimitiveComponent;

UCLASS(Abstract)
class ESCAPEIT_API AInteractableActor : public AActor, public IInteract
{
    GENERATED_BODY()
    
public:
    AInteractableActor();

protected:
    virtual void BeginPlay() override;

    // ===== INTERACTION COMPONENTS =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    UWidgetComponent* PromptWidget;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TSubclassOf<UUserWidget> InteractionPromptWidgetClass;
    
    UPROPERTY()
    UInteractionPromptWidget* InteractionPromptWidget;

    // ===== INTERACTION STATE =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bPlayerNearby;

    // ===== INTERACTION FUNCTIONS =====
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void OnInteractionBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void OnInteractionEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void ShowPrompt(bool bShow);
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void InitializePromptWidget();

    void NotifyPlayerControllerEnter(AActor* Player);
    void NotifyPlayerControllerLeave(AActor* Player);

public:
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void StartHoldInteraction();
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void UpdateHoldProgress(float Progress);
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void CancelHoldInteraction();
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void CompleteHoldInteraction();
    
    UFUNCTION(BlueprintCallable,Category="Interaction")
    virtual void UpdateWidgetRotation();
};