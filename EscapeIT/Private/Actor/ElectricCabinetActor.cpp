// ElectricCabinetActor.cpp
#include "Actor/ElectricCabinetActor.h"
#include "Components/TimelineComponent.h"
#include "GameInstance/PowerSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/NotificationWidget.h"
#include "UI/HUD/WidgetManager.h"

AElectricCabinetActor::AElectricCabinetActor()
{
    PrimaryActorTick.bCanEverTick = true;

    OpenAngle = 135.0f;
    bIsOpen = false;
    CachedPlayerController = nullptr;
}

void AElectricCabinetActor::BeginPlay()
{
    Super::BeginPlay();
}

void AElectricCabinetActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AElectricCabinetActor::CalculateDoorOpenDirection_Implementation(AActor* Interactor)
{
    Super::CalculateDoorOpenDirection_Implementation(Interactor);
}

void AElectricCabinetActor::Interact_Implementation(AActor* Interactor)
{
    IInteract::Interact_Implementation(Interactor);
    
    if (!bCanInteract)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cabinet cannot be interacted with"));
        return;
    }
    
    UPowerSystemManager* PowerSystem = GetGameInstance()->GetSubsystem<UPowerSystemManager>();
    if (!PowerSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("PowerSystemManager not found!"));
        return;
    }
    
    if (PowerSystem->IsPowerOn())
    {
        UE_LOG(LogTemp, Log, TEXT("Power is already on. No repair needed."));
        return;
    }
 
    if (!bIsOpen)
    {
        CachedPlayerController = Cast<APlayerController>(Interactor->GetInstigatorController());
        if (!CachedPlayerController)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot get PlayerController from Interactor"));
            return;
        }
        
        CalculateDoorOpenDirection_Implementation(Interactor);
        
        OpenDoor_Implementation();
        bIsOpen = true; 
        
        UE_LOG(LogTemp, Log, TEXT("Opening cabinet door..."));
        
        if (WidgetShowTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(WidgetShowTimerHandle);
        }

        GetWorld()->GetTimerManager().SetTimer(
            WidgetShowTimerHandle,
            this,
            &AElectricCabinetActor::OnDoorOpenFinished,
            WidgetShowDelay,
            false
        );
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Closing cabinet door..."));
        
        HidePuzzleWidget();
        
        if (WidgetShowTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(WidgetShowTimerHandle);
        }
        
        CloseDoor_Implementation();
        bIsOpen = false;
        
        CachedPlayerController = nullptr;
    }
}

void AElectricCabinetActor::OnDoorOpenFinished()
{
    UE_LOG(LogTemp, Log, TEXT("Door opened! Showing puzzle widget..."));
    
    if (CachedPlayerController && CachedPlayerController->IsValidLowLevel())
    {
        ShowPuzzleWidget(CachedPlayerController);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController is invalid when trying to show widget"));

        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            ShowPuzzleWidget(PC);
        }
    }
    
    CachedPlayerController = nullptr;
}

void AElectricCabinetActor::ShowPuzzleWidget(APlayerController* PlayerController)
{
    if (!ElectricCabinetWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ElectricCabinetWidgetClass not set!"));
        return;
    }
    
    if (!ElectricCabinetWidget)
    {
        ElectricCabinetWidget = CreateWidget<UElectricCabinetWidget>(PlayerController, ElectricCabinetWidgetClass);
        if (ElectricCabinetWidget)
        {
            ElectricCabinetWidget->OnPuzzleCompleted.AddDynamic(this, &AElectricCabinetActor::OnPuzzleCompleted);
            UE_LOG(LogTemp, Log, TEXT("ElectricCabinetWidget created successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create ElectricCabinetWidget"));
            return;
        }
    }
    
    if (ElectricCabinetWidget->IsInViewport())
    {
        UE_LOG(LogTemp, Warning, TEXT("Widget already in viewport"));
        return;
    }
    
    ElectricCabinetWidget->ShowAnimWidget();
    ElectricCabinetWidget->AddToViewport(100);
    
    PlayerController->SetInputMode(FInputModeUIOnly());
    PlayerController->bShowMouseCursor = true;
    
    UE_LOG(LogTemp, Log, TEXT("Puzzle widget shown successfully"));
}

void AElectricCabinetActor::HidePuzzleWidget()
{
    if (ElectricCabinetWidget && ElectricCabinetWidget->IsInViewport())
    {
      
        
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Puzzle widget hidden"));
    }
    ElectricCabinetWidget->SetVisibility(ESlateVisibility::Collapsed);
    ElectricCabinetWidget->HideAnimWidget();

}

void AElectricCabinetActor::OnPuzzleCompleted()
{
    UE_LOG(LogTemp, Log, TEXT("Puzzle completed! Restoring power..."));
    
    UPowerSystemManager* PowerSystem = GetGameInstance()->GetSubsystem<UPowerSystemManager>();
    if (PowerSystem)
    {
        PowerSystem->SetPowerState(true);
        bIsRepaired = true;
        
        AWidgetManager* WidgetManager = Cast<AWidgetManager>(GetWorld()->GetFirstPlayerController()->GetHUD());
        WidgetManager->GetNotificationWidget()->ShowNotification(FText::FromString(TEXT("Completed!")));
        
        FTimerHandle HidePuzzle;
        GetWorld()->GetTimerManager().SetTimer(HidePuzzle,this,&AElectricCabinetActor::HidePuzzleWidget,3.0f,false);
        
        FTimerHandle CloseTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            CloseTimerHandle,
            [this]()
            {
                if (bIsOpen)
                {
                    CloseDoor_Implementation();
                    bIsOpen = false;
                    UE_LOG(LogTemp, Log, TEXT("Cabinet door closed automatically"));
                }
            },
            4.0f,
            false
        );
        
        UE_LOG(LogTemp, Log, TEXT("Power restored successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PowerSystemManager not found when completing puzzle"));
    }
}