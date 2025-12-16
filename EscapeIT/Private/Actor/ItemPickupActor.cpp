
#include "Actor/ItemPickupActor.h"
#include "Actor/Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

AItemPickupActor::AItemPickupActor()
{
    PrimaryActorTick.bCanEverTick = true;

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

    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PromptWidget->SetVisibility(false);

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

    // Load mesh và data từ DataTable
    InitializeFromDataTable();

    ShowPrompt(false);
}

void AItemPickupActor::InitializeFromDataTable()
{
    FItemData RowData;
    if (GetItemData(RowData))
    {
        // Set mesh
        if (RowData.ItemMesh && MeshComponent)
        {
            MeshComponent->SetStaticMesh(RowData.ItemMesh);
        }

        // Cache item name cho prompt
        CachedItemName = RowData.ItemName;

        UE_LOG(LogTemp, Log, TEXT("ItemPickupActor: Initialized '%s' from DataTable"), *RowData.ItemName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ItemPickupActor: Failed to load data for ItemID '%s'"), *ItemID.ToString());
    }
}

void AItemPickupActor::Interact_Implementation(AActor* Interactor)
{
    PickupItem(Interactor);
}

void AItemPickupActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AItemPickupActor::PickupItem(AActor* Collector)
{
    if (!Collector)
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupItem: Collector is null"));
        return;
    }

    UInventoryComponent* Inventory = Collector->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupItem: Collector has no InventoryComponent!"));
        return;
    }

    FItemData ItemData;
    if (!GetItemData(ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("PickupItem: Invalid ItemID '%s'"), *ItemID.ToString());
        return;
    }

    if (Inventory->AddItem(ItemID, Quantity))
    {
        // Spawn particle effect
        if (PickupParticle)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                PickupParticle,
                GetActorLocation(),
                FRotator::ZeroRotator,
                FVector(1.0f)
            );
        }

        // Play sound
        TObjectPtr<USoundBase> SoundToPlay = ItemData.PickupSound ? ItemData.PickupSound : PickupSound;
        if (SoundToPlay)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                SoundToPlay,
                GetActorLocation()
            );
        }

        UE_LOG(LogTemp, Log, TEXT("PickupItem: Successfully picked up %d x %s"),
            Quantity, *ItemData.ItemName.ToString());

        // Destroy actor
        Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupItem: Failed to add item (inventory full or overweight)"));

        // TODO: Show notification to player
        // if (APlayerController* PC = Cast<APlayerController>(Collector->GetInstigatorController()))
        // {
        //     ShowInventoryFullNotification(PC);
        // }
    }
}

bool AItemPickupActor::CanBePickedUp(AActor* Collector) const
{
    if (!Collector)
    {
        return false;
    }

    UInventoryComponent* Inventory = Collector->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
    {
        return false;
    }

    return !Inventory->IsInventoryFull();
}

bool AItemPickupActor::GetItemData(FItemData& OutData) const
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("GetItemData: ItemDataTable not set!"));
        return false;
    }

    if (ItemID.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("GetItemData: ItemID is None!"));
        return false;
    }

    const FString ContextString = TEXT("GetItemData");
    FItemData* Data = ItemDataTable->FindRow<FItemData>(ItemID, ContextString);
    if (Data)
    {
        OutData = *Data;
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("GetItemData: ItemID '%s' not found in DataTable"), *ItemID.ToString());
    return false;
}

void AItemPickupActor::SetItemID(FName NewItemID)
{
    ItemID = NewItemID;
    InitializeFromDataTable();
}

void AItemPickupActor::OnInteractionBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    if (OtherActor->ActorHasTag(FName("Player")))
    {
        bPlayerNearby = true;
        ShowPrompt(true);

        if (bAutoPickup)
        {
            PickupItem(OtherActor);
        }

        UE_LOG(LogTemp, Log, TEXT("Player entered pickup range for '%s'"), *CachedItemName.ToString());
    }
}

void AItemPickupActor::OnInteractionEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    if (OtherActor->ActorHasTag(FName("Player")))
    {
        bPlayerNearby = false;
        ShowPrompt(false);

        UE_LOG(LogTemp, Log, TEXT("Player left pickup range"));
    }
}

void AItemPickupActor::ShowPrompt(bool bShow)
{
    if (PromptWidget)
    {
        PromptWidget->SetVisibility(bShow);
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