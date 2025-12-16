#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interact.h"
#include "Actor/Door/Door.h"
#include "DoorActor.generated.h"

UCLASS()
class ESCAPEIT_API ADoorActor : public ADoor, public IInteract
{
	GENERATED_BODY()

public:
	ADoorActor();

	// Interface function từ IInteractableInterface
	virtual void Interact_Implementation(AActor* Interactor) override;

private:
	void CalculateDoorOpenDirection(AActor* Interactor);

	virtual void OpenDoor_Implementation() override;
	virtual void CloseDoor_Implementation() override;
	
};