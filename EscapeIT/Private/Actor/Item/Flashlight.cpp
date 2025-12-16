
#include "Actor/Item/Flashlight.h"
#include "Components/SpotLightComponent.h"

AFlashlight::AFlashlight()
{
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
    
    MeshComponent->SetupAttachment(RootSceneComponent);
    
    SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComponent"));
    SpotLightComponent->SetupAttachment(RootSceneComponent);
    
    // Set initial spotlight properties
    SpotLightComponent->SetIntensity(5000.0f);
    SpotLightComponent->SetOuterConeAngle(45.0f);
    SpotLightComponent->SetAttenuationRadius(1000.0f);
    SpotLightComponent->SetVisibility(false);
    SpotLightComponent->SetHiddenInGame(true);
    SpotLightComponent->SetActive(false);
    
}

void AFlashlight::BeginPlay()
{
    Super::BeginPlay();
}

USpotLightComponent* AFlashlight::GetSpotLight() const
{
    return SpotLightComponent;
}
