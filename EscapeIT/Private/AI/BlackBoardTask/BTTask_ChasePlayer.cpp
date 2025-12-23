// BTTask_ChasePlayer.cpp
#include "AI/BlackBoardTask/BTTask_ChasePlayer.h"
#include "AI/NPC_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/NPC.h" 
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_ChasePlayer::UBTTask_ChasePlayer(FObjectInitializer const& ObjectInitializer) 
    : UBTTask_BlackboardBase{ ObjectInitializer }
{
    NodeName = TEXT("Chase Player");
    bNotifyTick = true; // Quan trọng: Cho phép TickTask được gọi
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* AICon = Cast<ANPC_AIController>(OwnerComp.GetAIOwner());
    if (!AICon)
        return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
        return EBTNodeResult::Failed;

    ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
    if (!NPC)
        return EBTNodeResult::Failed;

    // Lấy vị trí target từ blackboard (nên là Vector)
    FVector TargetLocation = BB->GetValueAsVector(GetSelectedBlackboardKey());

    if (TargetLocation.IsZero())
    {
        return EBTNodeResult::Failed;
    }
    
    bool bCanSeePlayer = BB->GetValueAsBool(TEXT("CanSeePlayer"));

    if (bCanSeePlayer)
    {
        if (auto* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
        {
            // Cập nhật vị trí người chơi liên tục
            TargetLocation = Player->GetActorLocation();
            BB->SetValueAsVector(GetSelectedBlackboardKey(), TargetLocation);
        }
        BB->SetValueAsBool(TEXT("IsPlayerBeingChased"), true);
    }
    else
    {
        BB->SetValueAsBool(TEXT("IsPlayerBeingChased"), false);
    }
    
    // Di chuyển đến vị trí
    FAIRequestID RequestID = AICon->MoveToLocation(
        TargetLocation,
        AcceptanceRadius,
        true,     // bStopOnOverlap
        true,     // bUsePathfinding
        false,    // bProjectDestinationToNavigation
        true,     // bCanStrafe
        nullptr,  // FilterClass
        true      // bAllowPartialPath
    );

    return RequestID.IsValid()
        ? EBTNodeResult::InProgress
        : EBTNodeResult::Failed;
}

void UBTTask_ChasePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    auto* AICon = Cast<ANPC_AIController>(OwnerComp.GetAIOwner());
    if (!AICon)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    EPathFollowingStatus::Type Status = AICon->GetMoveStatus();
    
    if (Status == EPathFollowingStatus::Type::Idle)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
    else if (Status == EPathFollowingStatus::Type::Paused)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
    
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB && BB->GetValueAsBool(TEXT("CanSeePlayer")))
    {
        if (auto* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
        {
            FVector NewLocation = Player->GetActorLocation();
            BB->SetValueAsVector(GetSelectedBlackboardKey(), NewLocation);
            
            AICon->MoveToLocation(
                NewLocation,
                AcceptanceRadius,
                true, true, false, true, nullptr, true
            );
        }
    }
}

EBTNodeResult::Type UBTTask_ChasePlayer::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ANPC_AIController* AICon = Cast<ANPC_AIController>(OwnerComp.GetAIOwner()))
    {
        AICon->StopMovement();
    }
    
    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->SetValueAsBool(TEXT("IsPlayerBeingChased"), false);
    }
    
    return EBTNodeResult::Aborted;
}