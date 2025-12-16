// Fill out your copyright notice in the Description page of Project Settings.



#include "AI/BlackBoardTask/BTTask_IncrementPathIndex.h"
#include "AI/NPC_AIController.h"
#include "AI/NPC.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

UBTTask_IncrementPathIndex::UBTTask_IncrementPathIndex(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = TEXT("Increment Path Index");
}

EBTNodeResult::Type UBTTask_IncrementPathIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//th? và l?y b? ?i?u khi?n c?a AI
	if (auto* const Cont = Cast<ANPC_AIController>(OwnerComp.GetAIOwner()))
	{
		//th? và l?y npc
		if (auto* const NPC = Cast<ANPC>(Cont->GetPawn()))
		{
			//th? và l?y blackboard
			if (auto* const BC = OwnerComp.GetBlackboardComponent())
			{
				//get number of the patrol point and min and max indices
				auto const NoOfPoints = NPC->GetPatrolPath()->Num();
				auto const MinIndex = 0;
				auto const MaxIndex = NoOfPoints - 1;
				auto Index = BC->GetValueAsInt(GetSelectedBlackboardKey());

				//change direction if we are at the first or last index if we are in bidirection mode
				if (bBiDirectional)
				{
					if (Index >= MaxIndex && Direction == EDirectionType::Forward)
					{
						Direction = EDirectionType::Reverse;
					}
					else if (Index == MinIndex && Direction == EDirectionType::Reverse)
					{
						Direction = EDirectionType::Forward;
					}
				}


				//write new value of index to blackboard
				BC->SetValueAsInt(GetSelectedBlackboardKey(), (Direction == EDirectionType::Forward ? ++Index : --Index) % NoOfPoints);

				//finish with success
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}
