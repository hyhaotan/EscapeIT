// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/MainMenuHUD.h"
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/MainMenu/MainMenuWidget.h"

void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();
    
    const auto WidgetMgr = UGameplayStatics::GetPlayerCameraManager(this,0);
    WidgetMgr->StartCameraFade(1.0f,0.0f,3.0f,FLinearColor::Black);
	
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle,[this]()
    {
        InitializeWidget();
        UGameplayStatics::PlaySound2D(this,AmbientSound);
    },4.0f,false);
}


void AMainMenuHUD::InitializeWidget()
{
    TObjectPtr<APlayerController> PC = UGameplayStatics::GetPlayerController(this, 0);
    PC->bShowMouseCursor = true;

    if (MainMenuWidgetClass)
    {
        MainMenuWidget = CreateWidget<UMainMenuWidget>(PC, MainMenuWidgetClass);
        if (MainMenuWidget)
        {
            MainMenuWidget->AddToViewport();

            // Setup input mode and focus
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            
            MainMenuWidget->ShowMainMenuWidget();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create MainMenuWidget"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MainMenuWidgetClass not assigned in HUD"));
    }
}

