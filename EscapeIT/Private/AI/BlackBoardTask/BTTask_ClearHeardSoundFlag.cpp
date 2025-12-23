// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackBoardTask/BTTask_ClearHeardSoundFlag.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearHeardSoundFlag::UBTTask_ClearHeardSoundFlag()
{
	NodeName = TEXT("Clear Heard Sound Flag");
}

EBTNodeResult::Type UBTTask_ClearHeardSoundFlag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!BB) return EBTNodeResult::Failed;
	
	BB->SetValueAsBool(HeardSoundKeyName,false);

	if (bClearLocation)
	{
		BB->ClearValue(LocationkeyName);
	}
	
	return EBTNodeResult::Succeeded;
}
