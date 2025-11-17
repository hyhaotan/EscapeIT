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

	bReplicates = true;
	SetReplicateMovement(true);
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