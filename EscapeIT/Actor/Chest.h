// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Interface/Interact.h"
#include "EscapeIT/Data/ItemData.h"
#include "Chest.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UTimelineComponent;
class UCurveFloat;
class UInventoryComponent;
class UNotificationWidget;

UCLASS()
class ESCAPEIT_API AChest : public AActor, public IInteract
{
    GENERATED_BODY()
    
public:    
    AChest();
    
    virtual void Tick(float DeltaTime) override;
    
    // IInteractInterface
    virtual void Interact_Implementation(AActor* Interactor) override;
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    bool CanBeOpenedWithKey(EKeyType InKeyType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void UnlockWithKey(AActor* Interactor, EKeyType InKeyType);

protected:
    virtual void BeginPlay() override;
    
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ChestMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* LidMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionBox;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTimelineComponent* OpenTimeline;
    
    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings")
    bool bRequiresKey = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings", meta = (EditCondition = "bRequiresKey"))
    EKeyType RequiredKeyType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings", meta = (EditCondition = "bRequiresKey"))
    bool bRequireKeyEquipped = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings", meta = (EditCondition = "bRequiresKey"))
    bool bConsumeKeyOnOpen = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Settings")
    TArray<FName> ChestItems;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Animation")
    UCurveFloat* ChestCurve;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Animation")
    float MaxLidRotation = 90.0f;
    
    // Sounds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Sound")
    USoundBase* OpenSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Sound")
    USoundBase* LockedSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest Sound")
    USoundBase* UnlockSound;
    
    // UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest UI")
    TSubclassOf<UNotificationWidget> NotificationWidgetClass;
    
    UPROPERTY()
    UNotificationWidget* NotificationWidget;
    
    // State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest State")
    bool bIsOpen = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest State")
    bool bIsLocked = true;
    
private:
    // Helper functions
    bool CheckCanOpenChest(AActor* Interactor, FString& OutFailReason);
    void OpenChest(AActor* Interactor);
    
    UFUNCTION()
    void UpdateLidRotation(float Value);
    
    UFUNCTION()
    void OnOpenFinished();
    
    void PlayChestSound(USoundBase* Sound);
    
    UInventoryComponent* GetInventoryFromActor(AActor* Actor) const;
};