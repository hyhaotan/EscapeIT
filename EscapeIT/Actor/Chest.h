// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Interface/Interact.h"
#include "EscapeIT/Data/ItemData.h"
#include "Chest.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UCurveFloat;
class UTimelineComponent;
class UInventoryComponent;
class UNotificationWidget;

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

    // Interface Interact
    virtual void Interact_Implementation(AActor* Interactor) override;

    // ============================================
    // COMPONENTS
    // ============================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ChestMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> LidMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> InteractionBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTimelineComponent> OpenTimeline;

    // ============================================
    // CHEST SETTINGS
    // ============================================
    
    // Loại key yêu cầu để mở chest này
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Settings")
    EKeysType RequiredKeyType;

    // Có cần key để mở không (một số chest không cần key)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Settings")
    bool bRequiresKey = true;

    // Có cần cầm key trên tay không (nếu false thì chỉ cần có trong inventory)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Settings")
    bool bRequireKeyEquipped = true;

    // Có tiêu hao key sau khi mở không
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Settings")
    bool bConsumeKeyOnOpen = false;

    // Curve cho animation mở nắp
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Animation")
    TObjectPtr<UCurveFloat> ChestCurve;

    // Góc xoay tối đa của nắp (độ)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Animation")
    float MaxLidRotation = 90.0f;

    // Các item bên trong chest (thiết lập trong editor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Contents")
    TArray<FName> ChestItems;

    // ============================================
    // STATE
    // ============================================
    UPROPERTY(BlueprintReadOnly, Category = "Chest|State")
    bool bIsOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Chest|State")
    bool bIsLocked = true;

    // ============================================
    // SOUNDS
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Audio")
    TObjectPtr<USoundBase> OpenSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Audio")
    TObjectPtr<USoundBase> LockedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Audio")
    TObjectPtr<USoundBase> UnlockSound;

private:
    bool CheckCanOpenChest(AActor* Interactor, FString& OutFailReason);
    
    void OpenChest(AActor* Interactor);
    
    UFUNCTION()
    void UpdateLidRotation(float Value);

    UFUNCTION()
    void OnOpenFinished();
    
    void PlayChestSound(USoundBase* Sound);
    
    UInventoryComponent* GetInventoryFromActor(AActor* Actor) const;
    
    UPROPERTY()
    TObjectPtr<UNotificationWidget> NotificationWidget;
    
    UPROPERTY(EditAnywhere,Category="Chest")
    TSubclassOf<UNotificationWidget> NotificationWidgetClass;
};