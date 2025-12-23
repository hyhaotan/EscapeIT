// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackBoardTask/BTTask_InvestigateSound.h"
#include "AI/NPC_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/NPC.h"
#include "AIController.h"

UBTTask_InvestigateSound::UBTTask_InvestigateSound(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Investigate Sound");
	
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_InvestigateSound::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* AICon = Cast<ANPC_AIController>(OwnerComp.GetAIOwner());
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}
	
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		return EBTNodeResult::Failed;
	}
	
	ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
	if (!NPC)
	{
		return EBTNodeResult::Failed;
	}
	
	FVector SoundLocation = BlackboardComponent->GetValueAsVector(GetSelectedBlackboardKey());

	if (SoundLocation.IsZero() || SoundLocation == FVector::ZeroVector)
	{
		return EBTNodeResult::Failed;
	}
	
	FAIRequestID RequestID = AICon->MoveToLocation(
		SoundLocation,
		AcceptanceRadius,
		true,
		true,
		false,
		true,
		nullptr,
		true);

	if (!RequestID.IsValid())
	{
		return EBTNodeResult::Failed;
	}
	
	BlackboardComponent->SetValueAsBool("IsInvestigating", true);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_InvestigateSound::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
    
	// Clear investigating flag
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsBool("IsInvestigating", false);
        
		// Clear sound location sau khi investigate xong
		// BB->ClearValue(GetSelectedBlackboardKey());
        
		UE_LOG(LogTemp, Warning, TEXT("InvestigateSound: Task finished - %s"), 
			TaskResult == EBTNodeResult::Succeeded ? TEXT("SUCCESS") : TEXT("FAILED"));
	}
}
