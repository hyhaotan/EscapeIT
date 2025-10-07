
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
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
    virtual void Tick(float DeltaTime) override;

public:
    // ============================================
    // COMPONENTS
    // ============================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UWidgetComponent* PromptWidget;

    // ============================================
    // PROPERTIES
    // ============================================

    // Item ID trong DataTable
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Quantity = 1;

    // Có tự động pickup khi chạm không
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bAutoPickup = false;

    // Interaction range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float InteractionRadius = 150.0f;

    // Visual effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|VFX")
    bool bRotateItem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|VFX")
    float RotationSpeed = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|VFX")
    bool bFloatItem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|VFX")
    float FloatAmplitude = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|VFX")
    float FloatSpeed = 2.0f;

    // Particles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|VFX")
    UParticleSystem* PickupParticle;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Audio")
    USoundBase* PickupSound;

    // Reference to DataTable
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    UDataTable* ItemDataTable;

    // ============================================
    // FUNCTIONS
    // ============================================

    // Pickup item
    UFUNCTION(BlueprintCallable, Category = "Item")
    void PickupItem(AActor* Collector);

    // Check có thể pickup không
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    bool CanBePickedUp(AActor* Collector) const;

    // Get item data
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    bool GetItemData(FItemData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Item")
    void SetItemDataByStruct(bool bShow);

protected:
    // ============================================
    // CALLBACKS
    // ============================================

    UFUNCTION()
    void OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Visual updates
    void UpdateVisualEffects(float DeltaTime);

    // Show/Hide prompt
    void ShowPrompt(bool bShow);

private:
    FVector InitialLocation;
    float FloatTimer = 0.0f;
    bool bPlayerNearby = false;
};