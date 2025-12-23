// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackBoardService/BTService_UpdatePlayerLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UBTService_UpdatePlayerLocation::UBTService_UpdatePlayerLocation()
{
	NodeName = TEXT("UpdatePlayerLocation");
	
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	
	bCallTickOnSearchStart = true;
	bTickIntervals = true;
}

void UBTService_UpdatePlayerLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UBlackboardComponent * BB = OwnerComp.GetBlackboardComponent();

	if (!BB) return;
	
	bool bCanSeePlayer = BB->GetValueAsBool(TEXT("CanSeePlayer"));
	if (!bCanSeePlayer) return;

	if (ACharacter * Char = UGameplayStatics::GetPlayerCharacter(GetWorld(),0))
	{
		FVector PlayerLocation = Char->GetActorLocation();
		
		BB->SetValueAsVector(GetSelectedBlackboardKey(),PlayerLocation);
	}
}
