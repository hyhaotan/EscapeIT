// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemPickupActor.h"
#include "GameFramework/Actor.h"
#include "Data/ItemData.h"
#include "Chest.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UTimelineComponent;
class UCurveFloat;
class USoundBase;
class UInventoryComponent;
class UNotificationWidget;

UCLASS()
class ESCAPEIT_API AChest : public AItemPickupActor
{
    GENERATED_BODY()
    
public:    
    AChest();

    virtual void Tick(float DeltaTime) override;
    
    virtual void Interact_Implementation(AActor* Interactor) override;

    // Public methods for key usage
    UFUNCTION(BlueprintCallable, Category = "Chest")
    bool CanBeOpenedWithKey(EKeyType InKeyType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void UnlockWithKey(AActor* Interactor, EKeyType InKeyType);

protected:
    virtual void BeginPlay() override;

    // ========================================================================
    // COMPONENTS
    // ========================================================================
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ChestMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* LidMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionBox;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTimelineComponent* OpenTimeline;

    // ========================================================================
    // CHEST SETTINGS
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings")
    EKeyType ChestType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings")
    bool bRequiresKey = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings",
        meta = (EditCondition = "bRequiresKey", EditConditionHides))
    EKeyType RequiredKeyType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings",
        meta = (EditCondition = "bRequiresKey", EditConditionHides))
    bool bRequireKeyEquipped = false;  // Có cần trang bị key không?
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings",
        meta = (EditCondition = "bRequiresKey", EditConditionHides))
    bool bConsumeKeyOnOpen = false;  // Key có bị tiêu hao không?

    // ========================================================================
    // CHEST CONTENTS
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Contents")
    TArray<FName> ChestItems;  // List of ItemIDs to give player

    // ========================================================================
    // CHEST STATE
    // ========================================================================
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest State")
    bool bIsOpen = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest State")
    bool bIsLocked = true;

    // ========================================================================
    // ANIMATION
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UCurveFloat* ChestCurve;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MaxLidRotation = 90.0f;

    // ========================================================================
    // AUDIO
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* OpenSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* LockedSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* UnlockSound;

    // ========================================================================
    // UI
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UNotificationWidget> NotificationWidgetClass;
    
    UPROPERTY()
    UNotificationWidget* NotificationWidget;

private:
    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================
    
    UFUNCTION()
    bool CheckCanOpenChest(AActor* Interactor, FString& OutFailReason);
    
    UFUNCTION()
    void OpenChest(AActor* Interactor);
    
    UFUNCTION()
    void UpdateLidRotation(float Value);
    
    UFUNCTION()
    void OnOpenFinished();
    
    UFUNCTION()
    void PlayChestSound(USoundBase* Sound);
    
    UFUNCTION()
    UInventoryComponent* GetInventoryFromActor(AActor* Actor) const;
    
    // Tìm ItemID của key phù hợp từ DataTable
    FName FindMatchingKeyItemID(UInventoryComponent* Inventory, EKeyType InKeyType) const;
};