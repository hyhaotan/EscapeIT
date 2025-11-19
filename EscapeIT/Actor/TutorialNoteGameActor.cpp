// Fill out your copyright notice in the Description page of Project Settings.

#include "TutorialNoteGameActor.h"
#include "EscapeIT/UI/TutorialNoteGameWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ATutorialNoteGameActor::ATutorialNoteGameActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    NoteMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoteMesh"));
    RootComponent = NoteMesh;
    
    DistanceFromPlayer = 150.0f; 
    MoveSpeed = 5.0f;
    bIsMoving = false;
}

void ATutorialNoteGameActor::BeginPlay()
{
    Super::BeginPlay();
    OriginalLocation = GetActorLocation();
    OriginalRotation = GetActorRotation();
}

void ATutorialNoteGameActor::Tick(float DeltaTime)
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
            
            // Chỉ show widget khi di chuyển đến camera
            if (bMovingToCamera)
            {
                ShowDocumentWidget();
                bMovingToCamera = false;
            }
        }
    }
}

void ATutorialNoteGameActor::Interact_Implementation(AActor* Interactable)
{
    IInteract::Interact_Implementation(Interactable);
    ShowDocument(this);
}

void ATutorialNoteGameActor::ShowDocument(AActor* Document)
{
    if (!Document) return;
    
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController) return;
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn) return;
    
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    FVector ForwardVector = CameraRotation.Vector();
    TargetLocation = CameraLocation + (ForwardVector * DistanceFromPlayer);
    
    FRotator LookAtRotation = (CameraLocation - TargetLocation).Rotation();
    SetActorRotation(LookAtRotation);
    
    bIsMoving = true;
    bMovingToCamera = true;
}

void ATutorialNoteGameActor::HideDocument(AActor* Document)
{
    if (DocumentWidget)
    {
        DocumentWidget->HideAnimation();
        
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle,[this]()
        {
            HideDocumentWidget();
            
            TargetLocation = OriginalLocation;
            SetActorRotation(OriginalRotation);
            bIsMoving = true;
            bMovingToCamera = false;
        },0.5f,false);
    }
}

void ATutorialNoteGameActor::ShowDocumentWidget()
{
    if (!DocumentWidgetClass) return;
    
    DocumentWidget = CreateWidget<UTutorialNoteGameWidget>(GetWorld(), DocumentWidgetClass);
    if (DocumentWidget)
    {
        // Set reference để widget có thể gọi lại actor
        DocumentWidget->SetNoteActorReference(this);
        
        APlayerController* PlayerCon = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PlayerCon)
        {
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(DocumentWidget->TakeWidget());
            PlayerCon->SetInputMode(InputMode);
            PlayerCon->bShowMouseCursor = true;
        }
        
        DocumentWidget->AddToViewport(999);
        DocumentWidget->SetVisibility(ESlateVisibility::Visible);
        
        DocumentWidget->ShowAnimation();
    }
}

void ATutorialNoteGameActor::HideDocumentWidget()
{
    if (DocumentWidget)
    {
        APlayerController* PlayerCon = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PlayerCon)
        {
            FInputModeGameOnly InputMode;
            PlayerCon->SetInputMode(InputMode);
            PlayerCon->bShowMouseCursor = false;
        }
        
        DocumentWidget->RemoveFromParent();
        DocumentWidget = nullptr;
    }
}