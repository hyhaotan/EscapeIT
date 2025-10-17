// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeIT/EscapeITCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/WidgetComponent.h"

ANPC::ANPC()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bIsWidgetVisible = false;
	bIsSearching = false;
	CurrentSearchIndex = 0;
	CurrentSpiralAngle = 0.0f;
	CurrentSpiralRadius = 0.0f;

	bReplicates = true;
	SetReplicateMovement(true);

	JumpScareCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("JumpScareCamera"));
	JumpScareCamera->SetupAttachment(GetMesh(), "head");
	JumpScareCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	JumpScareCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	InvestigationWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InvestigationWidgetComponent"));
	InvestigationWidgetComponent->SetupAttachment(RootComponent);
	InvestigationWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	InvestigationWidgetComponent->SetVisibility(false);
}

// Called when the game starts or when spawned
void ANPC::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UBehaviorTree* ANPC::GetBehaviorTree() const
{
	return Tree;
}

APaTrolPath* ANPC::GetPatrolPath() const
{
	return PatrolPath;
}

UAnimMontage* ANPC::GetMontage() const
{
	return Montage;
}

void ANPC::ToggleInvestigationWidgetVisibility(bool bCanSeePlayer)
{
	if (InvestigationWidgetComponent)
	{
		// Chỉ toggle khi trạng thái thay đổi
		if (bIsWidgetVisible != bCanSeePlayer)
		{
			bIsWidgetVisible = bCanSeePlayer;
			InvestigationWidgetComponent->SetVisibility(bIsWidgetVisible? true : false);
		}
	}
}

void ANPC::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (bUseJumpScareCamera)
	{
		// Custom camera position for jumpscare
		FVector CameraLocation = GetActorLocation() + FVector(0, 0, 60); // Slightly above NPC
		FRotator CameraRotation = GetActorRotation();

		// Look slightly down for dramatic effect
		CameraRotation.Pitch -= 15;

		OutResult.Location = CameraLocation;
		OutResult.Rotation = CameraRotation;
		OutResult.FOV = 90.0f; // Standard FOV

		return;
	}

	// Default camera behavior
	Super::CalcCamera(DeltaTime, OutResult);
}

void ANPC::SetIsSearching(bool bSearching)
{
    bIsSearching = bSearching;
}

bool ANPC::IsSearching() const
{
    return bIsSearching;
}

void ANPC::StartSearchPattern(const FVector& CenterLocation)
{
	SearchCenter = CenterLocation;
	CurrentSearchIndex = 0;
	CurrentSpiralAngle = 0.0f;
	CurrentSpiralRadius = 100.0f;
	SearchPoints.Empty();

	// Generate search points based on pattern type
	switch (SearchPattern)
	{
		case ESearchPatternType::Random:
			GenerateRandomSearchPoints();
			break;
		case ESearchPatternType::Spiral:
			GenerateSpiralSearchPoints();
			break;
		case ESearchPatternType::Grid:
			GenerateGridSearchPoints();
			break;
		case ESearchPatternType::LastKnown:
			SearchPoints.Add(CenterLocation);
			break;
	}
	UE_LOG(LogTemp, Warning, TEXT("[SearchPattern] Start at: %s"), *CenterLocation.ToString());
}

void ANPC::StopSearchPattern()
{
	bIsSearching = false;
	SearchPoints.Empty();
	CurrentSearchIndex = 0;
	UE_LOG(LogTemp, Warning, TEXT("[SearchPattern] Stop search"));
}

FVector ANPC::GetNextSearchLocation()
{
	if (!bIsSearching)
	{
		return GetActorLocation();
	}

	FVector NextPoint;
	switch (SearchPattern)
	{
	case ESearchPatternType::Random:
		NextPoint = GetNextRandomPoint();
		break;
	case ESearchPatternType::Spiral:
		NextPoint = GetNextSpiralPoint();
		break;
	case ESearchPatternType::Grid:
		NextPoint = GetNextGridPoint();
		break;
	case ESearchPatternType::LastKnown:
		NextPoint = SearchCenter;
		break;
	default:
		NextPoint = GetActorLocation();
		break;
	}
	UE_LOG(LogTemp, Warning, TEXT("[SearchPattern] Next point: %s"), *NextPoint.ToString());
	return NextPoint;
}

void ANPC::GenerateRandomSearchPoints()
{
	for (int32 i = 0; i < MaxSearchPoints; ++i)
	{
		float RandomAngle = FMath::RandRange(0.0f, 360.0f);
		float RandomRadius = FMath::RandRange(0.0f, SearchRadius);
		
		FVector Offset = FVector(
			FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomRadius,
			FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomRadius,
			0.0f
		);
		
		SearchPoints.Add(SearchCenter + Offset);
	}
}

void ANPC::GenerateSpiralSearchPoints()
{
	const float SpiralSpacing = 200.0f;
	const float AngleIncrement = 30.0f;
	
	for (int32 i = 0; i < MaxSearchPoints; ++i)
	{
		float Angle = i * AngleIncrement;
		float Radius = (i / 360.0f) * SpiralSpacing;
		
		if (Radius > SearchRadius)
		{
			break;
		}
		
		FVector Offset = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
			0.0f
		);
		
		SearchPoints.Add(SearchCenter + Offset);
	}
}

void ANPC::GenerateGridSearchPoints()
{
	int32 GridSize = FMath::CeilToInt(SearchRadius / GridCellSize);
	
	for (int32 x = -GridSize; x <= GridSize && SearchPoints.Num() < MaxSearchPoints; ++x)
	{
		for (int32 y = -GridSize; y <= GridSize && SearchPoints.Num() < MaxSearchPoints; ++y)
		{
			FVector Offset = FVector(x * GridCellSize, y * GridCellSize, 0.0f);
			if (Offset.Size() <= SearchRadius)
			{
				SearchPoints.Add(SearchCenter + Offset);
			}
		}
	}
}

FVector ANPC::GetNextRandomPoint() const
{
	if (SearchPoints.Num() == 0)
	{
		return GetActorLocation();
	}
	
	int32 Index = CurrentSearchIndex % SearchPoints.Num();
	return SearchPoints[Index];
}

FVector ANPC::GetNextSpiralPoint()
{
	CurrentSpiralAngle += 30.0f;
	CurrentSpiralRadius += 50.0f;
	
	if (CurrentSpiralRadius > SearchRadius)
	{
		CurrentSpiralRadius = 100.0f;
		CurrentSpiralAngle = 0.0f;
	}
	
	FVector Offset = FVector(
		FMath::Cos(FMath::DegreesToRadians(CurrentSpiralAngle)) * CurrentSpiralRadius,
		FMath::Sin(FMath::DegreesToRadians(CurrentSpiralAngle)) * CurrentSpiralRadius,
		0.0f
	);
	
	return SearchCenter + Offset;
}

FVector ANPC::GetNextGridPoint() const
{
	if (SearchPoints.Num() == 0)
	{
		return GetActorLocation();
	}
	
	int32 Index = CurrentSearchIndex % SearchPoints.Num();
	return SearchPoints[Index];
}

void ANPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANPC, bUseJumpScareCamera);
}

void ANPC::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (bUseJumpScareCamera && JumpScareCamera)
	{
		// Use the jumpscare camera's transform
		OutLocation = JumpScareCamera->GetComponentLocation();
		OutRotation = JumpScareCamera->GetComponentRotation();
	}
	else
	{
		// Fall back to the default implementation
		Super::GetActorEyesViewPoint(OutLocation, OutRotation);
	}
}
