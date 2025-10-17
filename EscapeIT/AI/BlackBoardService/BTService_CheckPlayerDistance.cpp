#include "BTService_CheckPlayerDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "EscapeIT/AI/NPC.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UBTService_CheckPlayerDistance::UBTService_CheckPlayerDistance()
{
    NodeName = TEXT("Check Player Distance");
    // Set interval để service chạy thường xuyên hơn
    Interval = 0.1f; // Kiểm tra mỗi 0.1 giây
    RandomDeviation = 0.05f;
}

void UBTService_CheckPlayerDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AICon = OwnerComp.GetAIOwner();
    ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
    UWorld* World = OwnerComp.GetWorld();
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AICon) return;
    if (!NPC) return;
    if (!World) return;
    if (!Player) return;
    if (!BlackboardComp) return;

    // Tính khoảng cách
    float Dist = FVector::Dist(NPC->GetActorLocation(), Player->GetActorLocation());
    bool bCanJump = Dist <= TriggerDistance;

    // Set value vào blackboard - đảm bảo key name chính xác
    BlackboardComp->SetValueAsBool(TEXT("bCanJumpScare"), bCanJump);

    // Debug: In ra giá trị trong blackboard
    bool CurrentValue = BlackboardComp->GetValueAsBool(TEXT("bCanJumpScare"));
}