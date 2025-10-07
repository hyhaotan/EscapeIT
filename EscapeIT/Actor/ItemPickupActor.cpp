#include "ItemPickupActor.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
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

    // Root component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetSimulatePhysics(false);

    // Interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(100.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Widget component cho prompt
    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PromptWidget->SetVisibility(false);

    // Default values
    InteractionRadius = 100.0f;
    Quantity = 1;
    bAutoPickup = false;
    bRotateItem = true;
    RotationSpeed = 45.0f;
    bFloatItem = true;
    FloatSpeed = 1.0f;
    FloatAmplitude = 8.0f;
    FloatTimer = 0.0f;
}

void AItemPickupActor::BeginPlay()
{
    Super::BeginPlay();

    InitialLocation = GetActorLocation();

    // Bind overlap events (safety: unbind then bind to avoid double bind)
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickupActor::OnInteractionBeginOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AItemPickupActor::OnInteractionEndOverlap);
    }

    // Load mesh từ DataTable nếu có
    FItemData RowData;
    if (GetItemData(RowData) && RowData.WorldMesh && MeshComponent)
    {
        MeshComponent->SetStaticMesh(RowData.WorldMesh);
    }

    // Hide prompt initially
    ShowPrompt(false);
}

void AItemPickupActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateVisualEffects(DeltaTime);
}

// ============================================
// PICKUP LOGIC
// ============================================
void AItemPickupActor::PickupItem(AActor* Collector)
{
    if (!Collector)
    {
        return;
    }

    // Tìm InventoryComponent
    UInventoryComponent* Inventory = Collector->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupItem: Collector has no InventoryComponent!"));
        return;
    }

    // Lấy item data
    FItemData ItemData;
    if (!GetItemData(ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("PickupItem: Invalid ItemID '%s'"), *ItemID.ToString());
        return;
    }

    // Thử add vào inventory
    if (Inventory->AddItem(ItemID, Quantity))
    {
        // Spawn particle effect
        if (PickupParticle)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupParticle, GetActorLocation());
        }

        // Play sound (sử dụng sound từ ItemData)
        USoundBase* SoundToPlay = ItemData.PickupSound ? ItemData.PickupSound : PickupSound;
        if (SoundToPlay)
        {
            UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, GetActorLocation());
        }

        UE_LOG(LogTemp, Log, TEXT("PickupItem: Picked up %d x %s"), Quantity, *ItemData.ItemName.ToString());

        // Destroy actor
        Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupItem: Failed to add item to inventory (full?)"));
        // TODO: Hiện notification "Inventory Full"
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

    // Check inventory có chỗ không
    return !Inventory->IsInventoryFull();
}

bool AItemPickupActor::GetItemData(FItemData& OutData) const
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("GetItemData: ItemDataTable not set!"));
        return false;
    }

    const FString ContextString = TEXT("GetItemData");
    FItemData* Data = ItemDataTable->FindRow<FItemData>(ItemID, ContextString);
    if (Data)
    {
        OutData = *Data;
        return true;
    }

    return false;
}

void AItemPickupActor::SetItemDataByStruct(bool bShow)
{
    // Nếu bạn có biến nội bộ lưu row hoặc ItemID, set tương ứng:
    //ItemStoredData = InData; // nếu bạn định lưu struct (không bắt buộc)
    // Cập nhật mesh nếu có
    //if (InData.WorldMesh && MeshComponent)
    //{
    //    MeshComponent->SetStaticMesh(InData.WorldMesh);
    //}
}

// ============================================
// OVERLAP CALLBACKS
// ============================================
void AItemPickupActor::OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    // Check if player (dựa trên tag "Player")
    if (OtherActor->ActorHasTag(FName("Player")))
    {
        bPlayerNearby = true;
        ShowPrompt(true);

        // Auto pickup nếu enabled
        if (bAutoPickup)
        {
            PickupItem(OtherActor);
        }

        UE_LOG(LogTemp, Log, TEXT("Player entered pickup range"));
    }
}

void AItemPickupActor::OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor)
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

// ============================================
// VISUAL EFFECTS
// ============================================
void AItemPickupActor::UpdateVisualEffects(float DeltaTime)
{
    // Rotation
    if (bRotateItem)
    {
        FRotator CurrentRotation = GetActorRotation();
        CurrentRotation.Yaw += RotationSpeed * DeltaTime;
        SetActorRotation(CurrentRotation);
    }

    // Floating
    if (bFloatItem)
    {
        FloatTimer += DeltaTime * FloatSpeed;
        float ZOffset = FMath::Sin(FloatTimer) * FloatAmplitude;
        FVector NewLocation = InitialLocation + FVector(0.0f, 0.0f, ZOffset);
        SetActorLocation(NewLocation);
    }
}

void AItemPickupActor::ShowPrompt(bool bShow)
{
    if (PromptWidget)
    {
        PromptWidget->SetVisibility(bShow);
    }
}