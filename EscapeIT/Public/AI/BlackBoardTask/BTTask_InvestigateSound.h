// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_InvestigateSound.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEIT_API UBTTask_InvestigateSound : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_InvestigateSound(FObjectInitializer const& ObjectInitializer);
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	float AcceptanceRadius  = 100.0f;
	
private:
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
};
