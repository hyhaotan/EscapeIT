
#include "EscapeITGameMode.h"

#include "ToolMenusLog.h"
#include "Actor/ElectricCabinetActor.h"
#include "Actor/HorrorDialogueSystem.h"
#include "GameInstance/PowerSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/WidgetManager.h"
#include "UI/StoryGameWidget.h"
#include "UI/SubtitleWidget.h"

AEscapeITGameMode::AEscapeITGameMode()
{
}

void AEscapeITGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDialogueSystem();
    
    PowerSystemManager = GetGameInstance()->GetSubsystem<UPowerSystemManager>();
    if (!PowerSystemManager)
    {
        UE_LOG(LogTemp, Error, TEXT("PowerSystemManager is null!"));
        return;
    }

    HideAllGameWidgets();
    FadeInAndShowStory();
    
    GetWorld()->GetTimerManager().SetTimer(
        PowerSystemManager->DelayPowerEvent,
        this,
        &AEscapeITGameMode::TriggerPowerEvent,
        PowerSystemManager->PowerOffDuration,
        false
    );
}

void AEscapeITGameMode::InitializeDialogueSystem()
{
    DialogueSystem = Cast<AHorrorDialogueSystem>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AHorrorDialogueSystem::StaticClass())
    );
    
    if (!DialogueSystem)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(TEXT("HorrorDialogueSystem"));
        SpawnParams.Owner = this;
        
        DialogueSystem = GetWorld()->SpawnActor<AHorrorDialogueSystem>(
            AHorrorDialogueSystem::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (DialogueSystem)
        {
            UE_LOG(LogTemp, Log, TEXT("DialogueSystem spawned successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn DialogueSystem!"));
            return;
        }
    }
    
    UDataTable* DialogueTable = LoadObject<UDataTable>(
        nullptr,
        TEXT("/Game/Data/DT_Dialogue.DT_Dialogue")
    );

    if (DialogueTable && DialogueSystem)
    {
        DialogueSystem->LoadDialoguesFromDataTable(DialogueTable);
        UE_LOG(LogTemp, Log, TEXT("Dialogue database loaded successfully"));
    }
    else if (!DialogueTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueTable not found at path: /Game/Data/DT_Dialogue"));
    }
}

void AEscapeITGameMode::HideAllGameWidgets()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World is null in HideAllGameWidgets"));
        return;
    }

    WidgetManager = Cast<AWidgetManager>(
        UGameplayStatics::GetActorOfClass(World, AWidgetManager::StaticClass())
    );

    if (WidgetManager)
    {
        WidgetManager->InitializeWidgets();
        WidgetManager->HideAllWidgets();
        UE_LOG(LogTemp, Log, TEXT("Widgets hidden successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WidgetManager not found in level"));
    }
}

void AEscapeITGameMode::FadeInAndShowStory()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController is null in FadeInAndShowStory"));
        return;
    }

    APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
    if (!CameraManager)
    {
        UE_LOG(LogTemp, Error, TEXT("CameraManager is null in FadeInAndShowStory"));
        return;
    }

    CameraManager->SetManualCameraFade(1.0f, FLinearColor::Black, false);
    
    FTimerHandle FadeTimer;
    GetWorldTimerManager().SetTimer(FadeTimer, [this, CameraManager]()
    {
        if (CameraManager)
        {
            CameraManager->StartCameraFade(1.0f, 0.0f, 1.5f, FLinearColor::Black, false, true);
        }

        // Show story widget
        ShowStoryGameWidget();

    }, 0.1f, false);
}

void AEscapeITGameMode::ShowStoryGameWidget()
{
    if (!StoryGameWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShowStoryGameWidget: StoryGameWidgetClass is null!"));
        return;
    }

    StoryGameWidget = CreateWidget<UStoryGameWidget>(GetWorld(), StoryGameWidgetClass);
    if (!StoryGameWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowStoryGameWidget: Failed to create StoryGameWidget instance."));
        return;
    }

    FString StoryStr = TEXT(
        "Today has no date. The clocks are an afterthought down here—frozen hands beneath dust and a faint smear where someone tried to rub a calendar clean. "
        "I woke on a tile floor with a headache like a drum and a key in my hand that did not belong to me. The corridor smelled of old paper and disinfectant gone rancid. "
        "Fluorescent lights hummed a tired tune; the sound threaded through the halls like a nervous laugh.\n\n"
        "There are names written in the margins of the building: Project Cycle, Consortium 11, and someone's hurried scrawl—remember Mina. I can feel gaps in my memory, "
        "like pages torn from a book I should have finished. Sometimes I see a child in the corner of my eye; sometimes I hear her calling my name in a voice I cannot place. "
        "The building answers with movement—the walls rearrange themselves, doors that were shut become ajar, shadows lengthen and fold into each other.\n\n"
        "This place was designed to hold memories, to bind them. Instead it trapped a promise. A promise is a small thing until it must be kept. I do not yet know whether "
        "I came here to keep it, to break it, or to be consumed by it.\n\n"
        "If you find these notes—if someone is reading what I cannot remember—learn this: truth is patient, but memory is jealous. It will show you what it wants, when it wants. "
        "The only way forward is to listen, collect what is scattered, and face what remembers you."
    );
    
    StoryGameWidget->SetStoryText(StoryStr);
    StoryGameWidget->AddToViewport(1000);
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(StoryGameWidget->TakeWidget());
        PC->SetInputMode(InputMode);
        
        UE_LOG(LogTemp, Log, TEXT("StoryGameWidget displayed successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController not found when showing story widget"));
    }
}

void AEscapeITGameMode::TriggerPowerEvent()
{
    AActor* Actor = UGameplayStatics::GetActorOfClass(
        GetWorld(),
        AElectricCabinetActor::StaticClass()
    );
    
    if (Actor && PowerSystemManager && PowerSystemManager->PowerOffSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            PowerSystemManager->PowerOffSound,
            Actor->GetActorLocation()
        );
    }
    else
    {
        if (!Actor)
            UE_LOG(LogTemp, Warning, TEXT("ElectricCabinetActor not found"));
        if (!PowerSystemManager)
            UE_LOG(LogTemp, Error, TEXT("PowerSystemManager is null"));
        if (PowerSystemManager && !PowerSystemManager->PowerOffSound)
            UE_LOG(LogTemp, Warning, TEXT("PowerOffSound is null"));
    }
    
    if (PowerSystemManager)
    {
        PowerSystemManager->CausePowerFailure();
    }
    
    if (DialogueSystem)
    {
        DialogueSystem->PlayDialogueSequence(FName(TEXT("PowerOutage")));
        UE_LOG(LogTemp, Log, TEXT("PowerOutage dialogue triggered"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueSystem is null in TriggerPowerEvent!"));
    }
}