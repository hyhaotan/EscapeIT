// Fill out your copyright notice in the Description page of Project Settings.


#include "PaTrolPath.h"

// Sets default values
APaTrolPath::APaTrolPath()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

FVector APaTrolPath::GetPatrolPoint(int const index) const
{
	return PatrolPoints[index];
}

int APaTrolPath::Num() const
{
	return PatrolPoints.Num();
}
