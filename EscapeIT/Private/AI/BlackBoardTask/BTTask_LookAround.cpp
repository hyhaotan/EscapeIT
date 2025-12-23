// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackBoardTask/BTTask_LookAround.h"
#include "AI/NPC_AIController.h"
#include "AI/NPC.h"

UBTTask_LookAround::UBTTask_LookAround(FObjectInitializer const& ObjectInitializer)
	: UBTTaskNode(ObjectInitializer)
{
	NodeName = TEXT("Look Around");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_LookAround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ElapsedTime = 0.0f;
    
	UE_LOG(LogTemp, Warning, TEXT("LookAround: Started looking around..."));
    
	return EBTNodeResult::InProgress;
}

void UBTTask_LookAround::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
    
	ElapsedTime += DeltaSeconds;
    
	if (auto* AICon = Cast<ANPC_AIController>(OwnerComp.GetAIOwner()))
	{
		if (auto* NPC = Cast<ANPC>(AICon->GetPawn()))
		{
			FRotator CurrentRotation = NPC->GetActorRotation();
			float NewYaw = CurrentRotation.Yaw + (DeltaSeconds * 45.0f); // Quay 45 độ/giây
			NPC->SetActorRotation(FRotator(0, NewYaw, 0));
		}
	}
    
	if (ElapsedTime >= LookAroundDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("LookAround: Finished"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}






