// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ClearHeardSoundFlag.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEIT_API UBTTask_ClearHeardSoundFlag : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_ClearHeardSoundFlag();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	FName HeardSoundKeyName = "HasHeardSound";
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	FName LocationkeyName = "LastHeardLocation";
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	bool bClearLocation = false;
};
