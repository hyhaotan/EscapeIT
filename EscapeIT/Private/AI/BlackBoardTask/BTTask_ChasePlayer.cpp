// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BlackBoardTask/BTTask_ChasePlayer.h"
#include "AI//NPC_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EscapeITCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AI//NPC.h" 

UBTTask_ChasePlayer::UBTTask_ChasePlayer(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = TEXT("Chase Player");
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // Lấy controller và pawn
    auto* AICon = Cast<ANPC_AIController>(OwnerComp.GetAIOwner());
    if (!AICon)
        return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(GetSelectedBlackboardKey()));
    ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
    if (!TargetActor || !NPC)
        return EBTNodeResult::Failed;

    // Cập nhật widget điều tra
    bool bCanSeePlayer = BB->GetValueAsBool(TEXT("CanSeePlayer"));

    // **Thêm phần fear:**
    if (bCanSeePlayer)
    {
        // 1) Notify thẳng tới component Fear của Character
        if (auto* HorrorChar = Cast<AEscapeITCharacter>(TargetActor))
        {
            //HorrorChar->NotifyChasedByMonster(1.5f);
        }

        // 2) (Tùy chọn) ghi luôn vào Blackboard một flag để Decorator khác hoặc Service biết
        BB->SetValueAsBool(TEXT("IsPlayerBeingChased"), true);
    }
    else
    {
        BB->SetValueAsBool(TEXT("IsPlayerBeingChased"), false);
    }

    // Chạy MoveToActor bình thường
    FAIRequestID RequestID = AICon->MoveToActor(
        TargetActor,
        100.f,    // AcceptanceRadius
        true,     // bStopOnOverlap
        true,     // bUsePathfinding
        false,    // bCanStrafe
        nullptr,  // FilterClass
        true      // bAllowPartialPath
    );

    return RequestID.IsValid()
        ? EBTNodeResult::InProgress
        : EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_ChasePlayer::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ANPC_AIController* AICon = Cast<ANPC_AIController>(OwnerComp.GetAIOwner()))
    {
        AICon->StopMovement();
    }
    return EBTNodeResult::Aborted;
}
