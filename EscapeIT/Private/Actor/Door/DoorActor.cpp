#include "Actor/Door/DoorActor.h"
#include "Components/TimelineComponent.h"
#include "EscapeITPlayerController.h"
#include "Kismet/GameplayStatics.h"

ADoorActor::ADoorActor()
{
    InteractionType = EInteractionType::Press;
    HoldDuration = 0.0f;
    
    OpenAngle = 90.0f;
    bIsOpen = false;

    UE_LOG(LogTemp, Log, TEXT("DoorActor created with PRESS interaction"));
}

void ADoorActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("DoorActor '%s' initialized - Type: PRESS, OpenAngle: %.1f"), 
        *GetName(), OpenAngle);
}

void ADoorActor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
    
    if (!bPlayerNearby || !Interactor)
    {
        UE_LOG(LogTemp, Warning, TEXT("DoorActor: Cannot interact (PlayerNearby: %s, Interactor: %s)"),
            bPlayerNearby ? TEXT("true") : TEXT("false"),
            Interactor ? TEXT("valid") : TEXT("null"));
        return;
    }

    if (bIsLocked)
    {
        OnDoorLocked(Interactor);
        return;
    }

    if (!bIsOpen)
    {
        CalculateDoorOpenDirection(Interactor);
        OpenDoor_Implementation();
    }
    else
    {
        CloseDoor_Implementation();
    }
    
    UE_LOG(LogTemp, Log, TEXT("DoorActor '%s' toggled: %s"), 
        *GetName(), 
        bIsOpen ? TEXT("OPENING") : TEXT("CLOSING"));
}

void ADoorActor::CalculateDoorOpenDirection_Implementation(AActor* Interactor)
{
    if (!Interactor)
    {
        UE_LOG(LogTemp, Warning, TEXT("CalculateDoorOpenDirection: Interactor is null"));
        return;
    }

    // Vector from door to player
    FVector DoorToPlayer = (Interactor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    // Door forward and right vectors
    FVector DoorForward = GetActorForwardVector();
    FVector DoorRight = GetActorRightVector();

    // Calculate dot products to determine player position relative to door
    float DotForward = FVector::DotProduct(DoorToPlayer, DoorForward);
    float DotRight = FVector::DotProduct(DoorToPlayer, DoorRight);

    // Determine door opening direction based on player position
    if (FMath::Abs(DotForward) > FMath::Abs(DotRight))
    {
        // Player is in front/back - rotate around Y axis
        if (DotForward > 0.0f)
        {
            // Player in front -> open to the right
            DoorRotationTarget.Yaw = -OpenAngle;
            UE_LOG(LogTemp, Log, TEXT("Door opening RIGHT (player in front)"));
        }
        else
        {
            // Player behind -> open to the left
            DoorRotationTarget.Yaw = OpenAngle;
            UE_LOG(LogTemp, Log, TEXT("Door opening LEFT (player behind)"));
        }
    }
    else
    {
        // Player is on left/right side
        if (DotRight > 0.0f)
        {
            // Player on right -> open to the right
            DoorRotationTarget.Yaw = OpenAngle;
            UE_LOG(LogTemp, Log, TEXT("Door opening RIGHT (player on right)"));
        }
        else
        {
            // Player on left -> open to the left
            DoorRotationTarget.Yaw = -OpenAngle;
            UE_LOG(LogTemp, Log, TEXT("Door opening LEFT (player on left)"));
        }
    }

    DoorRotationTarget.Pitch = 0.0f;
    DoorRotationTarget.Roll = 0.0f;
}

void ADoorActor::OpenDoor_Implementation()
{
    Super::OpenDoor_Implementation();

    if (!DoorTimeline)
    {
        UE_LOG(LogTemp, Error, TEXT("OpenDoor: DoorTimeline is null!"));
        return;
    }

    if (DoorTimeline->IsPlaying())
    {
        UE_LOG(LogTemp, Warning, TEXT("OpenDoor: Timeline already playing"));
        return;
    }

    bIsOpen = true;
    DoorTimeline->SetPlayRate(1.0f);
    DoorTimeline->PlayFromStart();
    
    UE_LOG(LogTemp, Log, TEXT("Door '%s' opening to angle: %.1f"), 
        *GetName(), DoorRotationTarget.Yaw);
}

void ADoorActor::CloseDoor_Implementation()
{
    Super::CloseDoor_Implementation();

    if (!DoorTimeline)
    {
        UE_LOG(LogTemp, Error, TEXT("CloseDoor: DoorTimeline is null!"));
        return;
    }

    if (DoorTimeline->IsPlaying())
    {
        UE_LOG(LogTemp, Warning, TEXT("CloseDoor: Timeline already playing"));
        return;
    }
    
    bIsOpen = false;
    DoorTimeline->SetPlayRate(-1.0f);
    DoorTimeline->Play();
    
    UE_LOG(LogTemp, Log, TEXT("Door '%s' closing"), *GetName());
}

void ADoorActor::OnDoorLocked(AActor* Interactor)
{
    UE_LOG(LogTemp, Log, TEXT("Door '%s' is locked!"), *GetName());
    
    if (LockedSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            LockedSound,
            GetActorLocation()
        );
    }
    
    if (APawn* Pawn = Cast<APawn>(Interactor))
    {
        if (AEscapeITPlayerController* PC = Cast<AEscapeITPlayerController>(Pawn->GetController()))
        {
            FString Message = RequiredKeyID.IsNone() ? 
                TEXT("Door is locked!") : 
                FString::Printf(TEXT("Requires %s"), *RequiredKeyID.ToString());
            
            PC->ShowNotification(Message);
        }
    }
}

void ADoorActor::SetLocked(bool bLocked)
{
    bIsLocked = bLocked;
    UE_LOG(LogTemp, Log, TEXT("Door '%s' locked state set to: %s"), 
        *GetName(), 
        bIsLocked ? TEXT("LOCKED") : TEXT("UNLOCKED"));
}

void ADoorActor::UnlockWithKey(FName KeyID)
{
    if (KeyID == RequiredKeyID || RequiredKeyID.IsNone())
    {
        SetLocked(false);
        UE_LOG(LogTemp, Log, TEXT("Door '%s' unlocked with key: %s"), 
            *GetName(), 
            *KeyID.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Door '%s' requires key: %s, but got: %s"), 
            *GetName(), 
            *RequiredKeyID.ToString(),
            *KeyID.ToString());
    }
}