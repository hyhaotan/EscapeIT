#include "Battery.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

ABattery::ABattery()
{
	// Set default ItemID
	ItemID = FName("Battery");
	Quantity = 1;
}

void ABattery::BeginPlay()
{
	Super::BeginPlay();
    
	// Ensure ItemID is set
	if (ItemID.IsNone())
	{
		ItemID = FName("Battery");
	}
    
	UE_LOG(LogTemp, Log, TEXT("Battery: Spawned with ItemID '%s'"), *ItemID.ToString());
}