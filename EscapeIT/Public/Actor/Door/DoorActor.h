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

	virtual void OpenDoor_Implementation() override;
	virtual void CloseDoor_Implementation() override;

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Door")
	void CalculateDoorOpenDirection(AActor* Interactor);
};