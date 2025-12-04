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
    
    // Create FlashlightComponent (this manages battery, toggle, etc.)
    FlashlightComponent = CreateDefaultSubobject<UFlashlightComponent>(TEXT("FlashlightComponent"));
    
    UE_LOG(LogTemp, Log, TEXT("AFlashlight: Components created"));
}

void AFlashlight::BeginPlay()
{
    Super::BeginPlay();
    
    if (FlashlightComponent)
    {
        // Bind to flashlight events
        FlashlightComponent->OnFlashlightToggled.AddDynamic(this, &AFlashlight::OnFlashlightToggled);
        FlashlightComponent->OnBatteryChanged.AddDynamic(this, &AFlashlight::OnBatteryChanged);
        FlashlightComponent->OnBatteryLow.AddDynamic(this, &AFlashlight::OnBatteryLow);
        FlashlightComponent->OnBatteryDepleted.AddDynamic(this, &AFlashlight::OnBatteryDepleted);
        
        // **CRITICAL: Give FlashlightComponent reference to its own actor**
        FlashlightComponent->InitializeFlashlight(this, SpotLightComponent);
        
        UE_LOG(LogTemp, Log, TEXT("AFlashlight: FlashlightComponent initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AFlashlight: FlashlightComponent is NULL!"));
    }
}

void AFlashlight::OnFlashlightToggled(bool bIsOn)
{
    UE_LOG(LogTemp, Log, TEXT("Flashlight toggled: %s"), bIsOn ? TEXT("ON") : TEXT("OFF"));
}

void AFlashlight::OnBatteryChanged(float Current, float Max)
{
    float Percentage = (Current / Max) * 100.0f;
    UE_LOG(LogTemp, Log, TEXT("Battery: %.1f%%"), Percentage);
}

void AFlashlight::OnBatteryLow()
{
    UE_LOG(LogTemp, Warning, TEXT("LOW BATTERY WARNING!"));
}

void AFlashlight::OnBatteryDepleted()
{
    UE_LOG(LogTemp, Error, TEXT("BATTERY DEPLETED!"));
}