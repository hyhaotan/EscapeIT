// Fill out your copyright notice in the Description page of Project Settings.

#include "DocumentActor.h"
#include "EscapeIT/Actor/Components/DocumentComponent.h"
#include "EscapeIT/UI/DocumentWidget.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

ADocumentActor::ADocumentActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    NoteMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoteMesh"));
    RootComponent = NoteMesh;
    
    DocumentComponent = CreateDefaultSubobject<UDocumentComponent>(TEXT("DocumentComponent"));
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    
    DistanceFromPlayer = 150.0f; 
    MoveSpeed = 5.0f;
    AnimationDuration = 0.5f;
    bIsMoving = false;
    bMovingToCamera = false;
    bEnableMovementAnimation = true;
    bCanPickup = false;
    bDestroyAfterRead = false;
    bIsPendingDestroy = false;
}

void ADocumentActor::BeginPlay()
{
    Super::BeginPlay();
    
    OriginalLocation = GetActorLocation();
    OriginalRotation = GetActorRotation();

    if (DocumentComponent)
    {
        DocumentComponent->OnDocumentRead.AddDynamic(this, &ADocumentActor::OnDocumentRead);
        DocumentComponent->OnDocumentClosed.AddDynamic(this, &ADocumentActor::OnDocumentClosed);
    }
}

void ADocumentActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MoveBackTimerHandle);
        World->GetTimerManager().ClearTimer(DestroyTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

void ADocumentActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMoving)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
        SetActorLocation(NewLocation);
        
        float Distance = FVector::Dist(NewLocation, TargetLocation);
        if (Distance < 1.0f)
        {
            bIsMoving = false;
            SetActorLocation(TargetLocation);
            
            if (bMovingToCamera)
            {
                ShowDocumentWidget();
                bMovingToCamera = false;
            }
        }
    }
}

// ============================================================================
// INTERACTION
// ============================================================================

void ADocumentActor::Interact_Implementation(AActor* Interactable)
{
    IInteract::Interact_Implementation(Interactable);
    
    // Prevent interaction if already being destroyed
    if (bIsPendingDestroy)
    {
        return;
    }
    
    ShowDocument(this);
}

void ADocumentActor::ShowDocument(AActor* Document)
{
    if (!Document || bIsPendingDestroy) return;
    
    // Validate data
    if (DocumentID.IsNone() || !DocumentDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("DocumentActor: Missing DocumentID or DataTable"));
        return;
    }

    if (!DocumentComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("DocumentActor: DocumentComponent is null!"));
        return;
    }

    // Cancel any pending movement
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MoveBackTimerHandle);
    }

    if (bEnableMovementAnimation)
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (!PlayerController) return;
        
        TargetLocation = CalculateCameraTargetLocation();
        
        FVector CameraLocation;
        FRotator CameraRotation;
        PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
        FRotator LookAtRotation = (CameraLocation - TargetLocation).Rotation();
        SetActorRotation(LookAtRotation);
        
        bIsMoving = true;
        bMovingToCamera = true;
    }
    else
    {
        ShowDocumentWidget();
    }
}

void ADocumentActor::HideDocument(AActor* Document)
{
    if (DocumentComponent)
    {
        DocumentComponent->CloseDocument();
    }
}

// ============================================================================
// WIDGET MANAGEMENT
// ============================================================================

void ADocumentActor::ShowDocumentWidget()
{
    if (!DocumentComponent || !DocumentDataTable || DocumentID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("DocumentActor: Cannot show document - missing data"));
        return;
    }

    bool bSuccess = false;
    
    if (DocumentWidgetClass)
    {
        bSuccess = DocumentComponent->DisplayDocumentWithWidget(DocumentID, DocumentDataTable, DocumentWidgetClass);
    }
    else
    {
        bSuccess = DocumentComponent->DisplayDocument(DocumentID, DocumentDataTable);
    }

    if (bSuccess)
    {
        DocumentWidget = DocumentComponent->GetCurrentDocumentWidget();
        
        // Set reference back to this actor for proper cleanup
        if (DocumentWidget)
        {
            DocumentWidget->SetDocumentActorReference(this);
        }
        
        UE_LOG(LogTemp, Log, TEXT("DocumentActor: Document %s displayed successfully"), *DocumentID.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DocumentActor: Failed to display document %s"), *DocumentID.ToString());
    }
}

void ADocumentActor::HideDocumentWidget()
{
    if (DocumentComponent)
    {
        DocumentComponent->CloseDocument();
    }
    
    DocumentWidget = nullptr;
}

// ============================================================================
// DOCUMENT EVENTS
// ============================================================================

void ADocumentActor::OnDocumentRead(FName ReadDocumentID)
{
    UE_LOG(LogTemp, Log, TEXT("DocumentActor: Document %s was read"), *ReadDocumentID.ToString());
    
    if (bCanPickup && InventoryComponent)
    {
        InventoryComponent->AddItem(ReadDocumentID, 1);
    }
}

void ADocumentActor::OnDocumentClosed()
{
    UE_LOG(LogTemp, Log, TEXT("DocumentActor: Document closed"));
    
    // Move back with proper timer handling
    if (bEnableMovementAnimation && !bIsPendingDestroy)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                MoveBackTimerHandle, 
                this, 
                &ADocumentActor::MoveBackToOriginalPosition,
                AnimationDuration,
                false
            );
        }
    }
    
    // Handle destruction
    if (bCanPickup && bDestroyAfterRead && !bIsPendingDestroy)
    {
        bIsPendingDestroy = true;
        
        float DestroyDelay = bEnableMovementAnimation ? (AnimationDuration + 0.5f) : AnimationDuration;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                DestroyTimerHandle,
                this,
                &ADocumentActor::SafeDestroy,
                DestroyDelay,
                false
            );
        }
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FVector ADocumentActor::CalculateCameraTargetLocation() const
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController) return GetActorLocation();
    
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    FVector ForwardVector = CameraRotation.Vector();
    return CameraLocation + (ForwardVector * DistanceFromPlayer);
}

void ADocumentActor::MoveBackToOriginalPosition()
{
    if (bIsPendingDestroy) return;
    
    TargetLocation = OriginalLocation;
    SetActorRotation(OriginalRotation);
    bIsMoving = true;
    bMovingToCamera = false;
}

void ADocumentActor::SafeDestroy()
{
    // Final cleanup before destroy
    if (DocumentWidget && DocumentWidget->IsInViewport())
    {
        DocumentWidget->RemoveFromParent();
    }
    
    DocumentWidget = nullptr;
    
    Destroy();
}