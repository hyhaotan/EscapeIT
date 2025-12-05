// Flashlight.cpp
#include "Flashlight.h"
#include "Components/SpotLightComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"

AFlashlight::AFlashlight()
{
    // Create SpotLight component
    SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComponent"));
    SpotLightComponent->SetupAttachment(MeshComponent);
    
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
    
    if (SpotLightComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("Flashlight BeginPlay: SpotLight initialized"));
        UE_LOG(LogTemp, Log, TEXT("  - Intensity: %.1f"), SpotLightComponent->Intensity);
        UE_LOG(LogTemp, Log, TEXT("  - Outer Cone: %.1f"), SpotLightComponent->OuterConeAngle);
        UE_LOG(LogTemp, Log, TEXT("  - Visibility: %s"), SpotLightComponent->IsVisible() ? TEXT("TRUE") : TEXT("FALSE"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Flashlight BeginPlay: SpotLight is NULL!"));
    }
}

USpotLightComponent* AFlashlight::GetSpotLight() const
{
    return SpotLightComponent;
}