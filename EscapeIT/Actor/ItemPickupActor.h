// ItemPickupActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Data/ItemData.h"
#include "ItemPickupActor.generated.h"

UCLASS()
class ESCAPEIT_API AItemPickupActor : public AActor
{
    GENERATED_BODY()

public:
    AItemPickupActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ============================================
    // COMPONENTS
    // ============================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UWidgetComponent* PromptWidget;

    // ============================================
    // ITEM DATA
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    class UDataTable* ItemDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 Quantity = 1;

    // ============================================
    // INTERACTION
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bAutoPickup = false;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    bool bPlayerNearby = false;

    // ============================================
    // EFFECTS
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* PickupParticle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class USoundBase* PickupSound;

    // ============================================
    // FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Item")
    void PickupItem(AActor* Collector);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    bool CanBePickedUp(AActor* Collector) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    bool GetItemData(FItemData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Item")
    void SetItemID(FName NewItemID);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    FText GetItemName() const;

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void ShowPrompt(bool bShow);

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
    // INTERNAL FUNCTIONS
    // ============================================

    void InitializeFromDataTable();

private:
    FVector InitialLocation;
    FText CachedItemName;
};