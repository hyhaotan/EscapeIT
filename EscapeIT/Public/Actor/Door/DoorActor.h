#pragma once

#include "CoreMinimal.h"
#include "Actor/Door/Door.h"
#include "DoorActor.generated.h"

UCLASS()
class ESCAPEIT_API ADoorActor : public ADoor
{
	GENERATED_BODY()

public:
	ADoorActor();
	
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	virtual void OnInteractionBeginOverlap_Implementation(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	virtual void OnInteractionEndOverlap_Implementation(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex) override;

	virtual void OpenDoor_Implementation() override;
	virtual void CloseDoor_Implementation() override;

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Door")
	void CalculateDoorOpenDirection(AActor* Interactor);
};