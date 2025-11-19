
#include "Flashlight.h"
#include "Components/SpotLightComponent.h"

AFlashlight::AFlashlight()
{
	SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComponent"));
	SpotLightComponent->SetupAttachment(MeshComponent, FName("SpotLight"));
	
}