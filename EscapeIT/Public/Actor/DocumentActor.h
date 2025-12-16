// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interact.h"
#include "DocumentActor.generated.h"

class UDocumentComponent;
class UDataTable;
class UDocumentWidget;
class UInventoryComponent;
struct FTimerHandle; // forward-declare timer handle

UCLASS()
class ESCAPEIT_API ADocumentActor : public AActor, public IInteract
{
    GENERATED_BODY()
    
public:    
    ADocumentActor();

    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    virtual void Interact_Implementation(AActor* Interactable) override;
    
    void ShowDocument(AActor* Document);
    void HideDocument(AActor* Document);

protected:
    virtual void BeginPlay() override;

    // ========================================================================
    // COMPONENTS
    // ========================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* NoteMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDocumentComponent* DocumentComponent; 
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;

    // ========================================================================
    // DOCUMENT DATA
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
    FName DocumentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
    UDataTable* DocumentDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document|UI")
    TSubclassOf<UDocumentWidget> DocumentWidgetClass;

    // ========================================================================
    // MOVEMENT SETTINGS
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DistanceFromPlayer;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bEnableMovementAnimation;

    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Movement")
    float AnimationDuration;
    // ========================================================================
    // PICKUP SETTINGS
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
    bool bCanPickup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document", 
              meta = (EditCondition = "bCanPickup", EditConditionHides))
    bool bDestroyAfterRead;

private:
    // ========================================================================
    // WIDGET MANAGEMENT
    // ========================================================================

    void ShowDocumentWidget();
    void HideDocumentWidget();

    UPROPERTY()
    UDocumentWidget* DocumentWidget;

    // ========================================================================
    // MOVEMENT STATE
    // ========================================================================
    
    FVector OriginalLocation;
    FRotator OriginalRotation;
    FVector TargetLocation;
    bool bIsMoving;
    bool bMovingToCamera;
    
    FTimerHandle MoveBackTimerHandle;
    FTimerHandle DestroyTimerHandle;
    
    bool bIsPendingDestroy;

    // ========================================================================
    // DOCUMENT EVENTS
    // ========================================================================

    UFUNCTION()
    void OnDocumentRead(FName ReadDocumentID);

    UFUNCTION()
    void OnDocumentClosed();

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================
    
    FVector CalculateCameraTargetLocation() const;
    void MoveBackToOriginalPosition();
    void SafeDestroy();
};
