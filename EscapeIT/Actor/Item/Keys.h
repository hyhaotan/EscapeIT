// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "EscapeIT/Data/ItemData.h"
#include "Keys.generated.h"

class AChest;
class UNotificationWidget;

UCLASS()
class ESCAPEIT_API AKeys : public AItemPickupActor
{
    GENERATED_BODY()
    
public:
    AKeys();

    // Override UseItem to implement key usage logic
    virtual void UseItem_Implementation() override;

protected:
    virtual void BeginPlay() override;

    // ========================================================================
    // KEY PROPERTIES
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Settings")
    EKeyType KeyType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Settings",
        meta = (ClampMin = "100.0", ClampMax = "1000.0", Units = "cm"))
    float InteractionRange = 300.0f;  // 3 meters default

    // ========================================================================
    // CHEST INTERACTION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Key")
    bool TryUnlockChest();
    
    UFUNCTION(BlueprintCallable, Category = "Key")
    AChest* FindChestInRange();
    
    UFUNCTION(BlueprintCallable, Category = "Key")
    AChest* FindNearestChest();

    // ========================================================================
    // DOOR INTERACTION (For future implementation)
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Key")
    bool TryUnlockDoor();
    
    UFUNCTION(BlueprintCallable, Category = "Key")
    AActor* FindDoorInRange();

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    void ShowWrongKeyNotification();
    void ShowNoTargetNotification();
};