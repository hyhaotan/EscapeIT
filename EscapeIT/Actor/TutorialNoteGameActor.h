// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Interface/Interact.h"
#include "TutorialNoteGameActor.generated.h"

class UTutorialNoteGameWidget;

UCLASS()
class ESCAPEIT_API ATutorialNoteGameActor : public AActor, public IInteract
{
	GENERATED_BODY()
    
public:    
	ATutorialNoteGameActor();

	virtual void Tick(float DeltaTime) override;
    
	virtual void Interact_Implementation(AActor* Interactable) override;
    
	void ShowDocument(AActor* Document);
	void HideDocument(AActor* Document);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* NoteMesh;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTutorialNoteGameWidget> DocumentWidgetClass;
    
	UPROPERTY()
	UTutorialNoteGameWidget* DocumentWidget;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DistanceFromPlayer;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed;

private:
	void ShowDocumentWidget();
	void HideDocumentWidget();
    
	FVector OriginalLocation;
	FRotator OriginalRotation;
	FVector TargetLocation;
	bool bIsMoving;
	bool bMovingToCamera;
};