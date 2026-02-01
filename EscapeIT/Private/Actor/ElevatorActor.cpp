#include "Actor/ElevatorActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"

AElevatorActor::AElevatorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    ElevatorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ElevatorRoot"));
    RootComponent = ElevatorRoot;
    
    ElevatorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ElevatorMesh"));
    ElevatorMesh->SetupAttachment(RootComponent);
    
    // Interaction box - ở phía trước cửa
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(ElevatorMesh);
    InteractionBox->SetBoxExtent(FVector(100.0f, 150.0f, 100.0f));
    InteractionBox->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));
    InteractionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    
    // Player detection box - bên trong
    PlayerDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerDetectionBox"));
    PlayerDetectionBox->SetupAttachment(ElevatorMesh);
    PlayerDetectionBox->SetBoxExtent(FVector(150.0f, 150.0f, 120.0f));
    PlayerDetectionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    
    // Setup prompt widget location
    if (PromptWidget)
    {
        PromptWidget->SetRelativeLocation(FVector(200.0f, 0.0f, 100.0f));
    }
    
    // Floor heights
    FloorHeights.Add(EFloorType::GroundFloor, 0.0f);
    FloorHeights.Add(EFloorType::Floor_1, 400.0f);
    FloorHeights.Add(EFloorType::Floor_2, 800.0f);
    FloorHeights.Add(EFloorType::Floor_3, 1200.0f);
    FloorHeights.Add(EFloorType::Floor_4, 1600.0f);
    FloorHeights.Add(EFloorType::Floor_5, 2000.0f);
    
    InteractionType = EInteractionType::Press;
    HoldDuration = 0.0f;
    InteractionPrompt = FText::FromString("Call Elevator");
}

void AElevatorActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Chỉ bind cho cả 2 boxes vào cùng 1 function
    if (InteractionBox)
    {
        InteractionBox->OnComponentBeginOverlap.AddDynamic(
            this, &AElevatorActor::OnInteractionBeginOverlap);
        InteractionBox->OnComponentEndOverlap.AddDynamic(
            this, &AElevatorActor::OnInteractionEndOverlap);
    }
    
    if (PlayerDetectionBox)
    {
        PlayerDetectionBox->OnComponentBeginOverlap.AddDynamic(
            this, &AElevatorActor::OnInteractionBeginOverlap);
        PlayerDetectionBox->OnComponentEndOverlap.AddDynamic(
            this, &AElevatorActor::OnInteractionEndOverlap);
    }
    
    StartPosition = GetActorLocation();
    
    UE_LOG(LogTemp, Log, TEXT("ElevatorActor: Initialized at floor %d"), 
        static_cast<int32>(CurrentFloor));
}

void AElevatorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateElevatorState(DeltaTime);
}

void AElevatorActor::UpdateElevatorState(float DeltaTime)
{
    StateTimer += DeltaTime;

    switch (ElevatorState)
    {
    case EElevatorState::Idle:
        break;
        
    case EElevatorState::OpeningDoor:
        if (StateTimer >= DoorOpenTime)
        {
            bDoorOpen = true;
            ElevatorState = EElevatorState::WaitingForPassenger;
            StateTimer = 0.0f;
            InteractionPrompt = FText::FromString("Select Floor");
            UE_LOG(LogTemp, Log, TEXT("Elevator: Door opened"));
        }
        break;
        
    case EElevatorState::WaitingForPassenger:
        if (StateTimer >= WaitTimeAfterDoorOpen && !bPlayerInside)
        {
            CloseDoor();
        }
        break;
        
    case EElevatorState::ClosingDoor:
        if (StateTimer >= DoorCloseTime)
        {
            bDoorOpen = false;
            
            if (CurrentFloor != TargetFloor)
            {
                ElevatorState = EElevatorState::Moving;
                MoveProgress = 0.0f;
                StateTimer = 0.0f;
                
                StartPosition = GetActorLocation();
                TargetPosition = StartPosition;
                TargetPosition.Z = GetFloorHeight(TargetFloor);
                
                UE_LOG(LogTemp, Log, TEXT("Elevator: Moving from floor %d to %d"), 
                    static_cast<int32>(CurrentFloor), 
                    static_cast<int32>(TargetFloor));
            }
            else
            {
                ElevatorState = EElevatorState::Idle;
                StateTimer = 0.0f;
            }
        }
        break;
        
    case EElevatorState::Moving:
        {
            float Distance = FVector::Distance(StartPosition, TargetPosition);
            if (Distance < 1.0f)
            {
                CurrentFloor = TargetFloor;
                ElevatorState = EElevatorState::Arrived;
                StateTimer = 0.0f;
                break;
            }
            
            float MoveTime = Distance / MoveSpeed;
            MoveProgress += DeltaTime / MoveTime;
            MoveProgress = FMath::Clamp(MoveProgress, 0.0f, 1.0f);
            
            float Alpha = FMath::InterpEaseInOut(0.0f, 1.0f, MoveProgress, 2.0f);
            FVector NewLocation = FMath::Lerp(StartPosition, TargetPosition, Alpha);
            SetActorLocation(NewLocation);
            
            if (MoveProgress >= 1.0f)
            {
                CurrentFloor = TargetFloor;
                ElevatorState = EElevatorState::Arrived;
                StateTimer = 0.0f;
                UE_LOG(LogTemp, Log, TEXT("Elevator: Arrived at floor %d"), 
                    static_cast<int32>(CurrentFloor));
            }
        }
        break;
        
    case EElevatorState::Arrived:
        OpenDoor();
        break;
    }
}

void AElevatorActor::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
    
    UE_LOG(LogTemp, Log, TEXT("Elevator: Interact called (State: %d, DoorOpen: %d)"), 
        static_cast<int32>(ElevatorState), bDoorOpen);
    
    if (ElevatorState == EElevatorState::Idle && !bDoorOpen)
    {
        UE_LOG(LogTemp, Log, TEXT("Elevator: Calling elevator..."));
        OpenDoor();
    }
}

void AElevatorActor::RequestFloor(int32 FloorNumber)
{
    UE_LOG(LogTemp, Warning, TEXT("Elevator: RequestFloor called - Floor %d"), FloorNumber);
    
    if (FloorNumber < 0 || FloorNumber > 5)
    {
        UE_LOG(LogTemp, Warning, TEXT("Elevator: Invalid floor number: %d"), FloorNumber);
        return;
    }
    
    if (ElevatorState != EElevatorState::WaitingForPassenger)
    {
        UE_LOG(LogTemp, Warning, TEXT("Elevator: Cannot request floor in state: %d"), 
            static_cast<int32>(ElevatorState));
        return;
    }
    
    EFloorType RequestedFloor = static_cast<EFloorType>(FloorNumber);
    
    if (RequestedFloor == CurrentFloor)
    {
        UE_LOG(LogTemp, Log, TEXT("Elevator: Already at floor %d"), FloorNumber);
        return;
    }
    
    TargetFloor = RequestedFloor;
    UE_LOG(LogTemp, Log, TEXT("Elevator: Floor %d requested - closing door"), FloorNumber);
    
    CloseDoor();
}

void AElevatorActor::OpenDoor()
{
    if (bDoorOpen || ElevatorState == EElevatorState::OpeningDoor)
        return;
    
    ElevatorState = EElevatorState::OpeningDoor;
    StateTimer = 0.0f;
    PlayDoorAnimation(DoorOpenAnim);
    UE_LOG(LogTemp, Log, TEXT("Elevator: Opening door"));
}

void AElevatorActor::CloseDoor()
{
    if (!bDoorOpen || ElevatorState == EElevatorState::ClosingDoor)
        return;
    
    ElevatorState = EElevatorState::ClosingDoor;
    StateTimer = 0.0f;
    PlayDoorAnimation(DoorCloseAnim);
    InteractionPrompt = FText::FromString("Call Elevator");
    UE_LOG(LogTemp, Log, TEXT("Elevator: Closing door"));
}

void AElevatorActor::PlayDoorAnimation(UAnimSequenceBase* Animation)
{
    if (ElevatorMesh && Animation)
    {
        ElevatorMesh->PlayAnimation(Animation, false);
    }
}

float AElevatorActor::GetFloorHeight(EFloorType Floor) const
{
    if (const float* Height = FloorHeights.Find(Floor))
    {
        return *Height;
    }
    return 0.0f;
}

// *** GỘP TẤT CẢ LOGIC VÀO 1 FUNCTION DUY NHẤT ***
void AElevatorActor::OnInteractionBeginOverlap_Implementation(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || !Cast<ACharacter>(OtherActor))
        return;
    
    // Check box nào trigger overlap
    if (OverlappedComponent == InteractionBox)
    {
        // InteractionBox - cho phép interact
        Super::OnInteractionBeginOverlap_Implementation(
            OverlappedComponent, OtherActor, OtherComp, 
            OtherBodyIndex, bFromSweep, SweepResult);
        
        UE_LOG(LogTemp, Log, TEXT("Elevator: Player entered interaction range"));
    }
    else if (OverlappedComponent == PlayerDetectionBox)
    {
        // PlayerDetectionBox - detect player bên trong
        bPlayerInside = true;
        UE_LOG(LogTemp, Log, TEXT("Elevator: Player entered elevator"));
    }
}

void AElevatorActor::OnInteractionEndOverlap_Implementation(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!OtherActor || !Cast<ACharacter>(OtherActor))
        return;
    
    // Check box nào trigger overlap
    if (OverlappedComponent == InteractionBox)
    {
        // InteractionBox - rời khỏi interaction range
        Super::OnInteractionEndOverlap_Implementation(
            OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
        
        UE_LOG(LogTemp, Log, TEXT("Elevator: Player left interaction range"));
    }
    else if (OverlappedComponent == PlayerDetectionBox)
    {
        // PlayerDetectionBox - rời khỏi elevator
        bPlayerInside = false;
        UE_LOG(LogTemp, Log, TEXT("Elevator: Player exited elevator"));
    }
}