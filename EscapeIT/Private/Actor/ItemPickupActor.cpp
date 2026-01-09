#include "Actor/ItemPickupActor.h"
#include "Actor/Components/InventoryComponent.h"
#include "EscapeITPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "UI/Inventory/InteractionPromptWidget.h"
#include "Camera/PlayerCameraManager.h"

AItemPickupActor::AItemPickupActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    InteractionType = EInteractionType::Hold;
    HoldDuration = 1.5f;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetSimulatePhysics(true);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(100.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    if (PromptWidget)
    {
        PromptWidget->SetupAttachment(MeshComponent);
        PromptWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    }

    InteractionRadius = 100.0f;
    Quantity = 1;
    bAutoPickup = false;
}

void AItemPickupActor::BeginPlay()
{
    Super::BeginPlay();

    InitialLocation = GetActorLocation();

    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickupActor::OnInteractionBeginOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AItemPickupActor::OnInteractionEndOverlap);
        InteractionSphere->SetSphereRadius(InteractionRadius);
    }

    InitializeFromDataTable();
}

void AItemPickupActor::InitializeFromDataTable()
{
    FItemData RowData;
    if (GetItemData(RowData))
    {
        if (RowData.ItemMesh && MeshComponent)
        {
            MeshComponent->SetStaticMesh(RowData.ItemMesh);
        }

        CachedItemName = RowData.ItemName;
    }
}

void AItemPickupActor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
    
    PickupItem(Interactor);
}

void AItemPickupActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bPlayerNearby && PromptWidget && PromptWidget->IsVisible())
    {
        UpdateWidgetRotation();
    }
}

void AItemPickupActor::PickupItem(AActor* Collector)
{
    if (!Collector) return;

    UInventoryComponent* Inventory = Collector->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    FItemData ItemData;
    if (!GetItemData(ItemData)) return;

    if (!Inventory->CanAddItem(ItemID, Quantity))
    {
        if (APawn* Pawn = Cast<APawn>(Collector))
        {
            if (AEscapeITPlayerController* PC = Cast<AEscapeITPlayerController>(Pawn->GetController()))
            {
                PC->ShowNotification(TEXT("Inventory full!"));
            }
        }
        return;
    }

    if (Inventory->AddItem(ItemID, Quantity))
    {
        PlayPickupEffects(ItemData);
        
        NotifyPlayerControllerItemRemoved();

        Destroy();
    }
}

void AItemPickupActor::PlayPickupEffects(const FItemData& ItemData)
{
    TObjectPtr<USoundBase> SoundToPlay = ItemData.PickupSound ? ItemData.PickupSound : PickupSound;
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            SoundToPlay,
            GetActorLocation()
        );
    }
}

bool AItemPickupActor::CanBePickedUp(AActor* Collector) const
{
    if (!Collector) return false;

    UInventoryComponent* Inventory = Collector->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return false;

    return Inventory->CanAddItem(ItemID, Quantity);
}

bool AItemPickupActor::GetItemData(FItemData& OutData) const
{
    if (!ItemDataTable) return false;

    if (ItemID.IsNone()) return false;

    const FString ContextString = TEXT("GetItemData");
    FItemData* Data = ItemDataTable->FindRow<FItemData>(ItemID, ContextString);
    if (Data) return true;

    return false;
}

void AItemPickupActor::SetItemID(FName NewItemID)
{
    ItemID = NewItemID;
    InitializeFromDataTable();
}

void AItemPickupActor::OnInteractionBeginOverlap_Implementation(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    Super::OnInteractionBeginOverlap_Implementation(
       OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    if (MeshComponent)
    {
        MeshComponent->SetRenderCustomDepth(true);
        MeshComponent->SetCustomDepthStencilValue(1);
    }

    if (bAutoPickup && bPlayerNearby)
    {
        PickupItem(OtherActor);
    }
}

void AItemPickupActor::OnInteractionEndOverlap_Implementation(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    Super::OnInteractionEndOverlap_Implementation(
        OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

    if (MeshComponent)
    {
        MeshComponent->SetRenderCustomDepth(false);
        MeshComponent->SetCustomDepthStencilValue(0);
    }
}

void AItemPickupActor::NotifyPlayerControllerItemRemoved()
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (PlayerPawn)
    {
        AEscapeITPlayerController* PC = Cast<AEscapeITPlayerController>(PlayerPawn->GetController());
        if (PC)
        {
            PC->OnInteractableDestroyed(this);
        }
    }
}

void AItemPickupActor::UseItem_Implementation()
{
    
}

FText AItemPickupActor::GetItemName() const
{
    return CachedItemName;
}

APawn* AItemPickupActor::GetPlayerPawn() const
{
    return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}