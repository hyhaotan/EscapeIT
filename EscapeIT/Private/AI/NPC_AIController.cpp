// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/NPC_AIController.h"
#include "AI/NPC.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EscapeITCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Hearing.h"

ANPC_AIController::ANPC_AIController(FObjectInitializer const& ObjectInitializer)
{
	SetupPerceptionSystem();
}

void ANPC_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (ANPC* const npc = Cast<ANPC>(InPawn))
	{
		if (UBehaviorTree* const tree = npc->GetBehaviorTree())
		{
			UBlackboardComponent* b;
			UseBlackboard(tree->BlackboardAsset, b);
			Blackboard = b;
			RunBehaviorTree(tree);
		}
	}
}

void ANPC_AIController::SetupPerceptionSystem()
{
	// Chỉ tạo một lần PerceptionComponent
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));

	// Thiết lập SightConfig
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 1000.f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 25.f;
		SightConfig->PeripheralVisionAngleDegrees = 90.f;
		SightConfig->SetMaxAge(5.f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}

	// Thiết lập HearConfig
	HearConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hear Config"));
	if (HearConfig)
	{
		HearConfig->HearingRange = 3000.f;
		HearConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearConfig->DetectionByAffiliation.bDetectNeutrals = true;

		GetPerceptionComponent()->ConfigureSense(*HearConfig);
	}

	// Đặt giác quan chính (dominant sense)
	GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());

	// ===== QUAN TRỌNG: CHỈ BIND 1 LẦN! =====
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ANPC_AIController::OnTargetDetected);
}

void ANPC_AIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	UE_LOG(LogTemp, Warning, TEXT("OnTargetDetected called for: %s"), *Actor->GetName());
    
	if (auto* const ch = Cast<AEscapeITCharacter>(Actor))
	{
		if (!GetBlackboardComponent())
		{
			UE_LOG(LogTemp, Error, TEXT("Blackboard Component is NULL!"));
			return;
		}
       
		if (Stimulus.Type.Name == "Default__AISense_Sight")
		{
			bool bWasSensed = Stimulus.WasSuccessfullySensed();
			GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", bWasSensed);
            
			UE_LOG(LogTemp, Warning, TEXT("👁 SIGHT: CanSeePlayer = %s"), 
				bWasSensed ? TEXT("TRUE") : TEXT("FALSE"));
		}

		if (Stimulus.Type.Name == "Default__AISense_Hearing")
		{
			GetBlackboardComponent()->SetValueAsVector("LastHeardLocation", Stimulus.StimulusLocation);

			GetBlackboardComponent()->SetValueAsBool("HasHeardSound", true);
            
			UE_LOG(LogTemp, Warning, TEXT("👂 HEARING: Sound at location %s"), 
				*Stimulus.StimulusLocation.ToString());

			#if WITH_EDITOR
			DrawDebugSphere(
				GetWorld(),
				Stimulus.StimulusLocation,
				100.f,
				12,
				FColor::Yellow,
				false,
				3.0f
			);
			#endif
		}
	}
}