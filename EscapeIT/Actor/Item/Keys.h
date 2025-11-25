
#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "EscapeIT/Data/ItemData.h"
#include "Keys.generated.h"

UCLASS()
class ESCAPEIT_API AKeys : public AItemPickupActor
{
	GENERATED_BODY()
    
public: 
	AKeys();

	virtual void UseItem_Implementation() override;
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
	EKeyType KeyType;
	
	UFUNCTION(BlueprintCallable, Category = "Key")
	EKeyType GetKeyType() const { return KeyType; }
	
	
private:
	bool TryUnlockDoor();
	
	AActor* FindDoorInRange();
};