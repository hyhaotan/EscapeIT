#include "Actor/Door/Door.h"
#include "Components/TimelineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Curves/CurveFloat.h"
#include "GameSystem/AudioManager.h"
#include "Components/WidgetComponent.h"

ADoor::ADoor()
{
    PrimaryActorTick.bCanEverTick = true;

    DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
    RootComponent = DoorPivot;

    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(DoorPivot);

    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(DoorFrame);

    DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
    
    InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
    InteractBox->SetupAttachment(RootComponent);
    InteractBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    AudioManager = CreateDefaultSubobject<UAudioManager>(TEXT("AudioManager"));
    
    if (PromptWidget)
    {
        PromptWidget->SetupAttachment(DoorPivot);
    }
    
    bIsOpen = false;
    AnimationDuration = 1.0f;
    OpenAngle = 90.0f;
}

void ADoor::BeginPlay()
{
    Super::BeginPlay();

    if (DoorTimeline && DoorCurve)
    {
        FOnTimelineFloat TimelineCallback;
        TimelineCallback.BindDynamic(this, &ADoor::UpdateDoorRotation);
        DoorTimeline->AddInterpFloat(DoorCurve, TimelineCallback);
    }

    if (InteractBox)
    {
        InteractBox->OnComponentBeginOverlap.AddDynamic(this, &ADoor::OnInteractionBeginOverlap);
        InteractBox->OnComponentEndOverlap.AddDynamic(this, &ADoor::OnInteractionEndOverlap);
    }
}

void ADoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bPlayerNearby && PromptWidget && PromptWidget->IsVisible())
    {
        UpdateWidgetRotation();
    }
}

void ADoor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);

    UE_LOG(LogTemp, Log, TEXT("ADoor::Interact - Override in child class"));
}

void ADoor::OnInteractionBeginOverlap_Implementation(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    Super::OnInteractionBeginOverlap_Implementation(
        OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
    
}

void ADoor::OnInteractionEndOverlap_Implementation(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    Super::OnInteractionEndOverlap_Implementation(
        OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

}

void ADoor::UpdateDoorRotation_Implementation(float Value)
{
    if (!Door) return;

    FRotator CurrentRotation = FRotator(
        DoorRotationTarget.Pitch * Value,
        DoorRotationTarget.Yaw * Value,
        DoorRotationTarget.Roll * Value
    );

    Door->SetRelativeRotation(CurrentRotation);
}

void ADoor::OpenDoor_Implementation()
{
    if (AudioManager)
    {
        AudioManager->PlayOpenDoorSound();
    }
}

void ADoor::CloseDoor_Implementation()
{
    if (AudioManager)
    {
        AudioManager->PlayCloseDoorSound();
    }
}