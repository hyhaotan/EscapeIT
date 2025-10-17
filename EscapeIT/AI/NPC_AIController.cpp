// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC_AIController.h"
#include "NPC.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EscapeIT/EscapeITCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Hearing.h"

ANPC_AIController::ANPC_AIController(FObjectInitializer const& ObjectInitializer)
{
	SetupPerceptionSystem();
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ANPC_AIController::OnTargetPerceptionUpdated);
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
		//HearConfig->bUseLoSHearing = false;
		HearConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearConfig->DetectionByAffiliation.bDetectNeutrals = true;

		GetPerceptionComponent()->ConfigureSense(*HearConfig);
	}

	// Đặt giác quan chính (dominant sense)
	GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());

	// Đăng ký sự kiện khi nhận diện mục tiêu
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ANPC_AIController::OnTargetDetected);
}

void ANPC_AIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	if (auto* const ch = Cast<AEscapeITCharacter>(Actor))
	{
		// Cập nhật trạng thái "CanSeePlayer" nếu kích thích là hình ảnh (thị giác)
		if (Stimulus.Type.Name == "Default__AISense_Sight")
		{
			GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
		}

		// Cập nhật vị trí âm thanh nếu kích thích là âm thanh (thính giác)
		if (Stimulus.Type.Name == "Default__AISense_Hearing")
		{
			GetBlackboardComponent()->SetValueAsVector("LastHeardLocation", Stimulus.StimulusLocation);
			MoveToSoundSource(Stimulus.StimulusLocation);
		}
	}
}

void ANPC_AIController::MoveToSoundSource(FVector const& Location)
{
	MoveToLocation(Location);
}

void ANPC_AIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (auto* NPC = Cast<ANPC>(GetPawn()))
	{
		const bool bCanSee = Stimulus.WasSuccessfullySensed();
		NPC->ToggleInvestigationWidgetVisibility(bCanSee);
	}
}
