// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_LookAround.generated.h"

UCLASS()
class ESCAPEIT_API UBTTask_LookAround : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_LookAround(FObjectInitializer const& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float LookAroundDuration = 3.0f;

private:
	float ElapsedTime = 0.0f;
};
