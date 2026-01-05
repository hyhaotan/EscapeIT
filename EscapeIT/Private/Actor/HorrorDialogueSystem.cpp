
#include "Actor/HorrorDialogueSystem.h"
#include "UI/SubtitleWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "UI/HUD/WidgetManager.h"

AHorrorDialogueSystem::AHorrorDialogueSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentIntensity = EHorrorIntensity::Calm;
}

void AHorrorDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    WidgetManager = Cast<AWidgetManager>(PlayerController->GetHUD());
    
    // Initialize default reactions if empty
    if (CalmReactions.Num() == 0)
    {
        InitializeDefaultReactions();
    }

    // Load from DataTable if provided
    if (DialogueDataTable)
    {
        LoadDialoguesFromDataTable(DialogueDataTable);
    }
}

void AHorrorDialogueSystem::PlayDialogueSequence(FName SequenceID)
{
    if (!GetReferenceSubtitleWidget())
    {
        UE_LOG(LogTemp, Warning, TEXT("SubtitleWidget is null!"));
        return;
    }

    FHorrorDialogueSequence* Sequence = DialogueSequences.Find(SequenceID);
    if (!Sequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence '%s' not found!"), *SequenceID.ToString());
        return;
    }

    // Check if already played (for one-time sequences)
    if (Sequence->bHasBeenPlayed)
    {
        // Check if any line is set to play only once
        bool bShouldSkip = false;
        for (const FHorrorDialogueLine& Line : Sequence->DialogueLines)
        {
            if (Line.bOnlyPlayOnce)
            {
                bShouldSkip = true;
                break;
            }
        }
        if (bShouldSkip) return;
    }

    // Convert horror lines to subtitle lines using helper function
    TArray<FSubtitleLine> SubtitleLines;
    for (const FHorrorDialogueLine& HorrorLine : Sequence->DialogueLines)
    {
        SubtitleLines.Add(HorrorLine.ToSubtitleLine());
        
        // Apply horror effects for this line
        // Store these to trigger when line plays
        // (In production, you'd want a callback system)
    }

    // Play sequence
    CurrentSequenceID = SequenceID;
    Sequence->bHasBeenPlayed = true;
    OnDialogueStarted.Broadcast(SequenceID); 
    
    GetReferenceSubtitleWidget()->DisplaySubtitleSequence(SubtitleLines);

    // Bind completion event (only once)
    if (!GetReferenceSubtitleWidget()->OnSubtitleSequenceComplete.Contains(this, FName("HandleSubtitleComplete")))
    {
        GetReferenceSubtitleWidget()->OnSubtitleSequenceComplete.AddDynamic(this, &AHorrorDialogueSystem::HandleSubtitleComplete);
    }

    // Block input if needed
    if (Sequence->bBlockPlayerInput && PlayerController)
    {
        PlayerController->SetIgnoreMoveInput(true);
        PlayerController->SetIgnoreLookInput(true);
    }
}

void AHorrorDialogueSystem::PlayQuickDialogue(
    const FText& Text,
    EHorrorDialogueType Type,
    EHorrorIntensity Intensity)
{
    if (!GetReferenceSubtitleWidget()) return;

    // Use helper function to create line with auto-effects
    FHorrorDialogueLine Line = UDialogueDataHelpers::MakeHorrorLine(
        FText::FromString(TEXT("You")),
        Text,
        nullptr,
        2.5f,
        Intensity,
        false
    );
    Line.DialogueType = Type;

    FSubtitleLine SubLine = Line.ToSubtitleLine();
    GetReferenceSubtitleWidget()->DisplaySubtitle(SubLine.Name, SubLine.Subtitle, SubLine.Voice, SubLine.Duration);
    
    PlayHorrorEffects(Line);
}

void AHorrorDialogueSystem::StopCurrentDialogue()
{
    if (GetReferenceSubtitleWidget())
    {
        GetReferenceSubtitleWidget()->StopSubtitleSequence();
    }

    // Unblock input
    if (PlayerController)
    {
        PlayerController->SetIgnoreMoveInput(false);
        PlayerController->SetIgnoreLookInput(false);
    }
}

void AHorrorDialogueSystem::PlayRandomReaction(EHorrorIntensity Intensity)
{
    TArray<FText>* ReactionArray = nullptr;

    switch (Intensity)
    {
        case EHorrorIntensity::Calm:      ReactionArray = &CalmReactions; break;
        case EHorrorIntensity::Nervous:   ReactionArray = &NervousReactions; break;
        case EHorrorIntensity::Scared:    ReactionArray = &ScaredReactions; break;
        case EHorrorIntensity::Terrified: ReactionArray = &TerrifiedReactions; break;
        case EHorrorIntensity::Panic:     ReactionArray = &PanicReactions; break;
    }

    if (ReactionArray && ReactionArray->Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, ReactionArray->Num() - 1);
        FText RandomReaction = (*ReactionArray)[RandomIndex];
        
        PlayQuickDialogue(RandomReaction, EHorrorDialogueType::PlayerReaction, Intensity);

        // Update intensity if changed
        if (CurrentIntensity != Intensity)
        {
            OnIntensityChanged.Broadcast(CurrentIntensity, Intensity);
            CurrentIntensity = Intensity;
        }
    }
}

void AHorrorDialogueSystem::PlayInvestigationDialogue(const FText& ObjectName, const FText& Description)
{
    if (!GetReferenceSubtitleWidget()) return;

    TArray<FSubtitleLine> Lines;

    // Line 1: "Hmm... [Object]"
    Lines.Add(UDialogueDataHelpers::MakeSubtitleLine(
        FText::FromString(TEXT("You")),
        FText::Format(FText::FromString(TEXT("Hmm... {0}")), ObjectName),
        nullptr, 2.0f
    ));

    // Line 2: Description
    Lines.Add(UDialogueDataHelpers::MakeSubtitleLine(
        FText::GetEmpty(),
        Description,
        nullptr, 3.0f, true
    ));

    GetReferenceSubtitleWidget()->DisplaySubtitleSequence(Lines);
}

void AHorrorDialogueSystem::PlayReadNoteDialogue(const FText& NoteTitle, const TArray<FText>& NoteContent)
{
    if (!GetReferenceSubtitleWidget()) return;

    TArray<FSubtitleLine> Lines;

    // Title line
    Lines.Add(UDialogueDataHelpers::MakeSubtitleLine(
        FText::FromString(TEXT("[Document]")),
        NoteTitle,
        nullptr, 2.0f
    ));

    // Content lines
    for (const FText& Content : NoteContent)
    {
        float Duration = FMath::Max(3.0f, Content.ToString().Len() / 20.0f);
        Lines.Add(UDialogueDataHelpers::MakeSubtitleLine(
            FText::GetEmpty(),
            Content,
            nullptr, Duration, true
        ));
    }

    GetReferenceSubtitleWidget()->DisplaySubtitleSequence(Lines);
}

void AHorrorDialogueSystem::PlayRadioMessage(const FText& Message, bool bIsDistorted)
{
    if (!GetReferenceSubtitleWidget()) return;

    // Play static sound
    if (RadioStaticSound && bIsDistorted)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), RadioStaticSound);
    }

    FSubtitleLine Line = UDialogueDataHelpers::MakeSubtitleLine(
        FText::FromString(TEXT("[Radio Static]")),
        Message,
        nullptr, 3.5f
    );
    
    GetReferenceSubtitleWidget()->DisplaySubtitle(Line.Name, Line.Subtitle, Line.Voice, Line.Duration);
}

void AHorrorDialogueSystem::PlayWhisperDialogue(const FText& WhisperText)
{
    if (!GetReferenceSubtitleWidget()) return;

    if (WhisperAmbientSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), WhisperAmbientSound, 0.3f);
    }

    FSubtitleLine Line = UDialogueDataHelpers::MakeSubtitleLine(
        FText::FromString(TEXT("???")),
        WhisperText,
        nullptr, 4.0f
    );
    
    GetReferenceSubtitleWidget()->DisplaySubtitle(Line.Name, Line.Subtitle, Line.Voice, Line.Duration);
}

bool AHorrorDialogueSystem::HasSequenceBeenPlayed(FName SequenceID) const
{
    const FHorrorDialogueSequence* Sequence = DialogueSequences.Find(SequenceID);
    return Sequence ? Sequence->bHasBeenPlayed : false;
}

void AHorrorDialogueSystem::ResetSequence(FName SequenceID)
{
    FHorrorDialogueSequence* Sequence = DialogueSequences.Find(SequenceID);
    if (Sequence)
    {
        Sequence->bHasBeenPlayed = false;
    }
}

bool AHorrorDialogueSystem::IsDialoguePlaying() const
{
    return GetReferenceSubtitleWidget() && GetReferenceSubtitleWidget()->IsPlayingSubtitle();
}

void AHorrorDialogueSystem::LoadDialoguesFromDataTable(UDataTable* DataTable)
{
    if (!DataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("DataTable is null!"));
        return;
    }

    TArray<FDialogueTableRow*> Rows;
    DataTable->GetAllRows<FDialogueTableRow>(TEXT("LoadDialogues"), Rows);

    for (FDialogueTableRow* Row : Rows)
    {
        if (Row)
        {
            FName ID = Row->DialogueSequence.SequenceID;
            DialogueSequences.Add(ID, Row->DialogueSequence);
            UE_LOG(LogTemp, Log, TEXT("Loaded dialogue: %s"), *ID.ToString());
        }
    }
}

void AHorrorDialogueSystem::PlayHorrorEffects(const FHorrorDialogueLine& Line)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Play breathing
    if (Line.bPlayBreathingSound && BreathingSound)
    {
        UGameplayStatics::PlaySound2D(World, BreathingSound, 0.5f);
    }

    // Play heartbeat
    if (Line.bPlayHeartbeatSound && HeartbeatSound)
    {
        UGameplayStatics::PlaySound2D(World, HeartbeatSound, 0.7f);
    }

    // Screen shake
    if (Line.bShakeScreen && PlayerController && CameraShakeClass)
    {
        PlayerController->ClientStartCameraShake(
            CameraShakeClass,
            Line.ScreenShakeIntensity
        );
    }

    // TODO: Implement light flicker
    if (Line.bFlickerLights)
    {
        // Your light flicker implementation
    }
}

void AHorrorDialogueSystem::HandleSubtitleComplete()
{
    OnDialogueCompleted.Broadcast(CurrentSequenceID);
    CurrentSequenceID = NAME_None;

    // Unblock input
    if (PlayerController)
    {
        PlayerController->SetIgnoreMoveInput(false);
        PlayerController->SetIgnoreLookInput(false);
    }
}

void AHorrorDialogueSystem::InitializeDefaultReactions()
{
    // CALM
    CalmReactions = {
        FText::FromString(TEXT("Hmm... interesting.")),
        FText::FromString(TEXT("I should check this out.")),
        FText::FromString(TEXT("What's this?")),
        FText::FromString(TEXT("Let me take a closer look."))
    };

    // NERVOUS
    NervousReactions = {
        FText::FromString(TEXT("Something doesn't feel right...")),
        FText::FromString(TEXT("I should be careful here.")),
        FText::FromString(TEXT("This place gives me the creeps.")),
        FText::FromString(TEXT("I don't like this..."))
    };

    // SCARED
    ScaredReactions = {
        FText::FromString(TEXT("What the hell was that?!")),
        FText::FromString(TEXT("Oh god... oh god...")),
        FText::FromString(TEXT("I need to get out of here!")),
        FText::FromString(TEXT("This can't be real..."))
    };

    // TERRIFIED
    TerrifiedReactions = {
        FText::FromString(TEXT("NO! Stay away from me!")),
        FText::FromString(TEXT("Please... somebody help me!")),
        FText::FromString(TEXT("I'm going to die here...")),
        FText::FromString(TEXT("It's coming! RUN!"))
    };

    // PANIC
    PanicReactions = {
        FText::FromString(TEXT("NO NO NO NO!")),
        FText::FromString(TEXT("GET AWAY! GET AWAY!")),
        FText::FromString(TEXT("I CAN'T... I CAN'T DO THIS!")),
        FText::FromString(TEXT("HELP! SOMEBODY HELP ME!"))
    };
}