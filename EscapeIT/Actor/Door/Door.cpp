

#include "Door.h"
#include "Components/TimelineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "EscapeIT//GameSystem/AudioSubsystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Root component là DoorPivot - điểm quay của cửa
	DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
	RootComponent = DoorPivot;

	// Door Frame gắn vào DoorPivot
	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	DoorFrame->SetupAttachment(DoorPivot);

	// Door gắn vào DoorFrame
	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(DoorFrame);

	// Timeline cho animation
	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
	DoorTimeline->RegisterComponent();

	bIsOpen = false;
	AnimationDuration = 1.0f;
	OpenAngle = 90.0f;

}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();

	if (DoorTimeline && DoorCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindDynamic(this, &ADoor::UpdateDoorRotation);
		DoorTimeline->AddInterpFloat(DoorCurve, TimelineCallback);
	}
	
	AudioSubsystem = GetGameInstance()->GetSubsystem<UAudioSubsystem>();
	
	if (AudioSubsystem)
	{
		AudioSubsystem->SetupAudioEffects();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AudioSubsystem NOT found!"));
	}
	
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADoor::UpdateDoorRotation_Implementation(float Value)
{
	
	if (!Door)
	{
		return;
	}

	// Interpolate từ 0 đến DoorRotationTarget dựa vào curve value
	FRotator CurrentRotation = FRotator(
		DoorRotationTarget.Pitch * Value,
		DoorRotationTarget.Yaw * Value,
		DoorRotationTarget.Roll * Value
	);

	Door->SetRelativeRotation(CurrentRotation);
}

void ADoor::OpenDoor_Implementation()
{
	if (AudioSubsystem && AudioSubsystem->OpenDoorSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,AudioSubsystem->OpenDoorSound,GetActorLocation(), 1.0f);
	}
}

void ADoor::CloseDoor_Implementation()
{
	if (AudioSubsystem && AudioSubsystem->CloseDoorSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,AudioSubsystem->CloseDoorSound,GetActorLocation(), 1.0f);
	}
}