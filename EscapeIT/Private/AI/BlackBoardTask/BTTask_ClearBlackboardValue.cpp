// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackBoardTask/BTTask_ClearBlackboardValue.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearBlackboardValue::UBTTask_ClearBlackboardValue(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Clear Blackboard Value");
}

EBTNodeResult::Type UBTTask_ClearBlackboardValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	FName KeyName = GetSelectedBlackboardKey();
	BB->ClearValue(KeyName);
	
	return EBTNodeResult::Succeeded;
}
