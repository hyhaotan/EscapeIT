// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interact.h"
#include "Data/InteractionTypes.h"
#include "InteractableActor.generated.h"

class UWidgetComponent;
class UInteractionPromptWidget;

UCLASS()
class ESCAPEIT_API AInteractableActor : public AActor, public IInteract
{
	GENERATED_BODY()
	
public:	
	AInteractableActor();

	// Interaction Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EInteractionType InteractionType = EInteractionType::Hold;

	// Hold Interaction Duration (only used if InteractionType is Hold or Both)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "InteractionType == EInteractionType::Hold || InteractionType == EInteractionType::Both"))
	float HoldDuration = 1.0f;

	// Interaction Interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor);

	// Overlap Events
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void OnInteractionBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	virtual void OnInteractionEndOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Widget Management
	void ShowPrompt(bool bShow);
	void UpdateWidgetRotation();

	// Hold Interaction Methods
	void StartHoldInteraction();
	void UpdateHoldProgress(float Progress);
	void CancelHoldInteraction();
	void CompleteHoldInteraction();

	// Press Interaction Methods
	void OnPressInteraction();
	void ExecutePressInteraction();

	// Getters
	FORCEINLINE EInteractionType GetInteractionType() const { return InteractionType; }
	FORCEINLINE float GetHoldDuration() const { return HoldDuration; }

protected:
	virtual void BeginPlay() override;

	// Widget Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	UWidgetComponent* PromptWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

	UPROPERTY()
	UInteractionPromptWidget* InteractionPromptWidget;

	// State
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bPlayerNearby;

private:
	void InitializePromptWidget();
	void NotifyPlayerControllerEnter(AActor* Player);
	void NotifyPlayerControllerLeave(AActor* Player);
};