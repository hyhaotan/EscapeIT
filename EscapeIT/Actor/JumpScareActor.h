// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JumpScareActor.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;

UCLASS()
class ESCAPEIT_API AJumpScareActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJumpScareActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh | Skeletal")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh | Static")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

public:
	UFUNCTION()
	USkeletalMeshComponent* GetSkeletalMesh() { return SkeletalMesh; }

	UFUNCTION()
	UStaticMeshComponent* GetStaticMesh() { return StaticMesh; }
};
