#include "DoorActor.h"
#include "Components/TimelineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"
#include "Kismet/KismetMathLibrary.h"

ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	OpenAngle = 90.0f;
	bIsOpen = false;
}

void ADoorActor::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	// Tính toán hướng mở cửa dựa vào vị trí của người chơi
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

void ADoorActor::CalculateDoorOpenDirection(AActor* Interactor)
{
	// Vector từ door đến player
	FVector DoorToPlayer = (Interactor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

	// Forward vector của door (hướng mặt trước)
	FVector DoorForward = GetActorForwardVector();

	// Right vector của door (hướng phải)
	FVector DoorRight = GetActorRightVector();

	// Tính dot product để xác định player ở phía nào
	float DotForward = FVector::DotProduct(DoorToPlayer, DoorForward);
	float DotRight = FVector::DotProduct(DoorToPlayer, DoorRight);

	// Xác định hướng mở cửa
	// Nếu player ở phía trước (DotForward > 0), door mở về phía sau (Yaw âm)
	// Nếu player ở phía sau (DotForward < 0), door mở về phía trước (Yaw dương)

	if (FMath::Abs(DotForward) > FMath::Abs(DotRight))
	{
		// Player ở phía trước/sau - xoay theo trục Y
		if (DotForward > 0.0f)
		{
			// Player ở phía trước -> mở sang phải (hoặc trái tùy preference)
			DoorRotationTarget.Yaw = -OpenAngle;
		}
		else
		{
			// Player ở phía sau -> mở sang trái
			DoorRotationTarget.Yaw = OpenAngle;
		}
	}
	else
	{
		// Player ở phía trái/phải
		if (DotRight > 0.0f)
		{
			// Player ở phía phải -> mở sang phải
			DoorRotationTarget.Yaw = OpenAngle;
		}
		else
		{
			// Player ở phía trái -> mở sang trái
			DoorRotationTarget.Yaw = -OpenAngle;
		}
	}

	DoorRotationTarget.Pitch = 0.0f;
	DoorRotationTarget.Roll = 0.0f;
}

void ADoorActor::OpenDoor_Implementation()
{
	Super::OpenDoor_Implementation();

	if (!DoorTimeline || DoorTimeline->IsPlaying())
	{
		return;
	}

	bIsOpen = true;
	DoorTimeline->SetPlayRate(1.0f);
	DoorTimeline->PlayFromStart();
}

void ADoorActor::CloseDoor_Implementation()
{
	Super::CloseDoor_Implementation();

	if (!DoorTimeline || DoorTimeline->IsPlaying())
	{
		return;
	}

	bIsOpen = false;
	DoorTimeline->SetPlayRate(-1.0f);
	DoorTimeline->Play();
}
