#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Interface/Interact.h"
#include "EscapeIT/Actor/Door/Door.h"
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
};