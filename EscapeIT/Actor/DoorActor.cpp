#include "DoorActor.h"
#include "Components/TimelineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"

ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	RootComponent = DoorFrame;

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(DoorFrame);

	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
	DoorTimeline->RegisterComponent();

	bIsOpen = false;
}

void ADoorActor::BeginPlay()
{
	Super::BeginPlay();

	// Setup Timeline nếu có Curve
	if (DoorTimeline && DoorCurve)
	{
		// Bind callback function khi timeline update
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindDynamic(this, &ADoorActor::UpdateDoorRotation);
		DoorTimeline->AddInterpFloat(DoorCurve, TimelineCallback);
	}
}

void ADoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADoorActor::Interact_Implementation(AActor* Interactor)
{
	if (!bIsOpen)
	{
		OpenDoor();
	}
	else
	{
		CloseDoor();
	}
}

void ADoorActor::OpenDoor()
{
	if (!DoorTimeline || DoorTimeline->IsPlaying())
	{
		return;
	}

	bIsOpen = true;

	// Reset timeline và phát từ đầu (0 -> 1)
	DoorTimeline->SetPlayRate(1.0f);
	DoorTimeline->PlayFromStart();
}

void ADoorActor::CloseDoor()
{
	if (!DoorTimeline || DoorTimeline->IsPlaying())
	{
		return;
	}

	bIsOpen = false;

	// Phát timeline ngược (1 -> 0)
	DoorTimeline->SetPlayRate(-1.0f);
	DoorTimeline->Play();
}

void ADoorActor::UpdateDoorRotation(float Value)
{
	if (!Door)
	{
		return;
	}

	// Interpolate từ 0 đến DoorRot dựa vào giá trị từ curve (0.0 -> 1.0)
	FRotator CurrentRotation = FRotator(
		DoorRot.Pitch * Value,
		DoorRot.Yaw * Value,
		DoorRot.Roll * Value
	);

	Door->SetRelativeRotation(CurrentRotation);
}