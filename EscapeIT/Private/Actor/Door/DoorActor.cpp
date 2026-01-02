#include "Actor/Door/DoorActor.h"
#include "Components/TimelineComponent.h"

ADoorActor::ADoorActor()
{
    OpenAngle = 90.0f;
    bIsOpen = false;
}

void ADoorActor::Interact_Implementation(AActor* Interactor)
{
    if (!bIsOpen)
    {
        CalculateDoorOpenDirection(Interactor);
        OpenDoor_Implementation();
    }
    else
    {
        CloseDoor_Implementation();
    }
}

void ADoorActor::CalculateDoorOpenDirection_Implementation(AActor* Interactor)
{
    if (!Interactor) return;

    // Vector from door to player
    FVector DoorToPlayer = (Interactor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    // Door forward and right vectors
    FVector DoorForward = GetActorForwardVector();
    FVector DoorRight = GetActorRightVector();

    // Calculate dot products to determine player position
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
        }
        else
        {
            // Player behind -> open to the left
            DoorRotationTarget.Yaw = OpenAngle;
        }
    }
    else
    {
        // Player is on left/right side
        if (DotRight > 0.0f)
        {
            // Player on right -> open to the right
            DoorRotationTarget.Yaw = OpenAngle;
        }
        else
        {
            // Player on left -> open to the left
            DoorRotationTarget.Yaw = -OpenAngle;
        }
    }

    DoorRotationTarget.Pitch = 0.0f;
    DoorRotationTarget.Roll = 0.0f;
}

void ADoorActor::OpenDoor_Implementation()
{
    Super::OpenDoor_Implementation();

    if (!DoorTimeline || DoorTimeline->IsPlaying()) return;

    bIsOpen = true;
    DoorTimeline->SetPlayRate(1.0f);
    DoorTimeline->PlayFromStart();
}

void ADoorActor::CloseDoor_Implementation()
{
    Super::CloseDoor_Implementation();

    if (!DoorTimeline || DoorTimeline->IsPlaying()) return;
    
    bIsOpen = false;
    DoorTimeline->SetPlayRate(-1.0f);
    DoorTimeline->Play();
}