// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Interface/Interact.h"
#include "EscapeIT/Actor/Item/Keys.h"
#include "Chest.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UDataTable;
class USoundBase;
class UParticleSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChestOpened, AChest*, Chest, AActor*, Opener);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChestLocked, AChest*, Chest, AActor*, Interactor);

UCLASS()
class ESCAPEIT_API AChest : public AActor, public IInteract
{
    GENERATED_BODY()
    
public: 
    AChest();

protected:
    virtual void BeginPlay() override;

public: 
    virtual void Tick(float DeltaTime) override;

    // ============================================
    // COMPONENTS
    // ============================================
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ChestBaseMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ChestLidMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> InteractionSphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWidgetComponent> PromptWidget;

    // ============================================
    // CHEST SETTINGS
    // ============================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Lock Settings")
    bool bRequiresKey;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Lock Settings", 
        meta = (EditCondition = "bRequiresKey", EditConditionHides))
    EKeyType RequiredKeyType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Lock Settings", 
        meta = (EditCondition = "bRequiresKey", EditConditionHides))
    FName RequiredKeyID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Lock Settings")
    bool bConsumeKeyOnUse;
    
    UPROPERTY(BlueprintReadOnly, Category = "Chest|State")
    bool bIsOpen;
    
    UPROPERTY(BlueprintReadOnly, Category = "Chest|State")
    bool bIsLocked;

    // ============================================
    // LOOT SETTINGS
    // ============================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Loot")
    TArray<FName> LootItems;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Loot")
    TMap<FName, int32> LootItemsWithQuantity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Loot")
    TObjectPtr<UDataTable> ItemDataTable;

    // ============================================
    // ANIMATION SETTINGS
    // ============================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Animation")
    float OpenAngle;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Animation")
    float OpenSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Animation")
    UCurveFloat* OpenCurve;

    // ============================================
    // AUDIO & EFFECTS
    // ============================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Audio")
    TObjectPtr<USoundBase> OpenSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Audio")
    TObjectPtr<USoundBase> LockedSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Audio")
    TObjectPtr<USoundBase> UnlockSound;

    // ============================================
    // DELEGATES
    // ============================================
    
    UPROPERTY(BlueprintAssignable, Category = "Chest|Events")
    FOnChestOpened OnChestOpened;
    
    UPROPERTY(BlueprintAssignable, Category = "Chest|Events")
    FOnChestLocked OnChestLocked;

    // ============================================
    // INTERACTION INTERFACE
    // ============================================
    
    virtual void Interact_Implementation(AActor* Interactor) override;

    // ============================================
    // PUBLIC FUNCTIONS
    // ============================================
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    bool TryOpenChest(AActor* Opener);
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    bool HasRequiredKey(AActor* Interactor) const;
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void UnlockChest();
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void OpenChest(AActor* Opener);
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void SpawnLoot();
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void GiveLootToPlayer(AActor* Player);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chest")
    bool IsChestOpen() const { return bIsOpen; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chest")
    bool IsChestLocked() const { return bIsLocked; }
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void ShowPrompt(bool bShow);
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    FText GetInteractionPrompt() const;

protected:
    // ============================================
    // OVERLAP CALLBACKS
    // ============================================
    
    UFUNCTION()
    void OnInteractionBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);
    
    // ============================================
    // PROTECTED FUNCTIONS
    // ============================================
    
    void InitializeChest();
    void PlayOpenAnimation();
    void ConsumeKeyFromInventory(AActor* Player);
    
    UFUNCTION()
    void HandleOpenTimelineProgress(float Value);
    
    UFUNCTION()
    void OnOpenTimelineFinished();

private:
    // Animation state
    bool bIsAnimating;
    float CurrentLidAngle;
    float AnimationProgress;
    FRotator InitialLidRotation;
    UTimelineComponent* OpenTimeline;
    
    // Player tracking
    bool bPlayerNearby;
    AActor* NearbyPlayer;
};