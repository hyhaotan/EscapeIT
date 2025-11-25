// Fill out your copyright notice in the Description page of Project Settings.

#include "DocumentComponent.h"
#include "EscapeIT/UI/DocumentWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

UDocumentComponent::UDocumentComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    bIsDocumentOpen = false;
    bTriggerHorrorOnRead = false;
    SanityLossOnRead = 0.0f;
    HorrorEventDelay = 3.0f;
    CurrentReadingTime = 0.0f;
    bIsTrackingReadTime = false;
    CurrentDocumentID = NAME_None;
}

void UDocumentComponent::BeginPlay()
{
    Super::BeginPlay();
    CachedPlayerController = UGameplayStatics::GetPlayerController(this, 0);
}

void UDocumentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all resources
    CleanupResources();
    Super::EndPlay(EndPlayReason);
}

void UDocumentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDocumentOpen && bIsTrackingReadTime)
    {
        CurrentReadingTime += DeltaTime;
    }
}

// ============================================================================
// DOCUMENT MANAGEMENT
// ============================================================================

bool UDocumentComponent::DisplayDocument(FName ItemID, UDataTable* DataTable)
{
    return DisplayDocumentWithWidget(ItemID, DataTable, DefaultDocumentWidgetClass);
}

bool UDocumentComponent::DisplayDocumentWithWidget(FName ItemID, UDataTable* DataTable, TSubclassOf<UDocumentWidget> WidgetClass)
{
    // Validation
    if (ItemID.IsNone() || !DataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("DocumentComponent: Invalid ItemID or DataTable"));
        return false;
    }

    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("DocumentComponent: No widget class provided"));
        return false;
    }

    // Load document data
    if (!LoadDocumentData(ItemID, DataTable))
    {
        return false;
    }

    // Verify this is a document
    if (CurrentDocumentData.ItemType != EItemType::Document)
    {
        UE_LOG(LogTemp, Warning, TEXT("DocumentComponent: Item %s is not a document"), *ItemID.ToString());
        return false;
    }

    // Close existing document first
    if (bIsDocumentOpen)
    {
        UE_LOG(LogTemp, Log, TEXT("DocumentComponent: Closing existing document before opening new one"));
        CloseDocument();
        
        // Wait a frame for cleanup
        FTimerHandle DelayTimer;
        GetWorld()->GetTimerManager().SetTimer(DelayTimer, [this, ItemID, DataTable, WidgetClass]()
        {
            DisplayDocumentWithWidget(ItemID, DataTable, WidgetClass);
        }, 0.1f, false);
        
        return true;
    }

    // Get player controller
    if (!CachedPlayerController)
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(this, 0);
    }

    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("DocumentComponent: No player controller found"));
        return false;
    }

    // Create widget
    CurrentDocumentWidget = CreateWidget<UDocumentWidget>(CachedPlayerController, WidgetClass);
    if (!CurrentDocumentWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("DocumentComponent: Failed to create document widget"));
        return false;
    }

    // Setup widget
    CurrentDocumentWidget->SetDocumentComponentReference(this);
    CurrentDocumentWidget->SetupDocument(CurrentDocumentData);
    CurrentDocumentWidget->AddToViewport(100);

    // Update state
    bIsDocumentOpen = true;
    CurrentDocumentID = ItemID;
    SetComponentTickEnabled(true);
    StartReadingTimeTracking();

    // Play effects
    PlayOpenSound();
    CurrentDocumentWidget->ShowAnimation();

    // Start ambient sound
    if (ReadingAmbientSound)
    {
        PlayAmbientSound();
    }

    // Set input mode
    SetInputModeUI();

    // Mark as read
    MarkDocumentAsRead(ItemID);

    // Broadcast events
    OnDocumentRead.Broadcast(ItemID);
    OnDocumentContentChanged.Broadcast(CurrentDocumentData.DocumentContent, CurrentDocumentData.DocumentImage);

    // Apply effects
    if (SanityLossOnRead > 0.0f)
    {
        ApplySanityLossOnRead(SanityLossOnRead);
    }

    // Schedule horror event
    if (bTriggerHorrorOnRead && !HorrorEventID.IsNone())
    {
        ScheduleHorrorEvent();
    }

    // Special effects for important documents
    if (CurrentDocumentData.DocumentType == EDocumentType::FinalNote ||
        CurrentDocumentData.DocumentType == EDocumentType::DiaryPage)
    {
        PlayDiscoveryEffect();
    }

    UE_LOG(LogTemp, Log, TEXT("DocumentComponent: Successfully opened document %s"), *ItemID.ToString());
    return true;
}

void UDocumentComponent::CloseDocument()
{
    if (!bIsDocumentOpen)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("DocumentComponent: Closing document %s (Read time: %.2fs)"), 
           *CurrentDocumentID.ToString(), CurrentReadingTime);

    // Stop tracking
    StopReadingTimeTracking();

    // Play sound
    PlayCloseSound();

    // Handle widget animation and removal
    if (CurrentDocumentWidget && IsValid(CurrentDocumentWidget))
    {
        CurrentDocumentWidget->HideAnimation();
        
        // Get actual animation duration
        float AnimDuration = CurrentDocumentWidget->GetFadeOutDuration();
        
        FTimerHandle RemoveWidgetTimer;
        GetWorld()->GetTimerManager().SetTimer(RemoveWidgetTimer, [this]()
        {
            if (CurrentDocumentWidget && IsValid(CurrentDocumentWidget))
            {
                if (CurrentDocumentWidget->IsInViewport())
                {
                    CurrentDocumentWidget->RemoveFromParent();
                }
                CurrentDocumentWidget = nullptr;
            }
        }, AnimDuration, false);
    }

    // Stop ambient sound
    StopAmbientSound();

    // Restore input mode
    SetInputModeGame();

    // Clear horror event timer
    ClearHorrorEventTimer();

    // Broadcast close event
    OnDocumentClosed.Broadcast();

    // Reset state
    ResetDocumentState();
}

void UDocumentComponent::MarkDocumentAsRead(FName DocumentID)
{
    if (!DocumentID.IsNone() && !ReadDocuments.Contains(DocumentID))
    {
        ReadDocuments.Add(DocumentID);
        UE_LOG(LogTemp, Log, TEXT("DocumentComponent: Marked %s as read. Total: %d"), 
               *DocumentID.ToString(), ReadDocuments.Num());
    }
}

bool UDocumentComponent::HasReadDocument(FName DocumentID) const
{
    return ReadDocuments.Contains(DocumentID);
}

// ============================================================================
// AUDIO/VISUAL EFFECTS
// ============================================================================

void UDocumentComponent::PlayOpenSound()
{
    USoundBase* SoundToPlay = DocumentOpenSound;
    
    if (!SoundToPlay && CurrentDocumentData.UseSound)
    {
        SoundToPlay = CurrentDocumentData.UseSound;
    }

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySound2D(this, SoundToPlay);
    }
}

void UDocumentComponent::PlayCloseSound()
{
    if (DocumentCloseSound)
    {
        UGameplayStatics::PlaySound2D(this, DocumentCloseSound);
    }
}

void UDocumentComponent::PlayDiscoveryEffect()
{
    if (DocumentDiscoverySound)
    {
        UGameplayStatics::PlaySound2D(this, DocumentDiscoverySound);
    }

    // TODO: Add visual effects
    // - Camera shake
    // - Screen flash
    // - Particle effects
}

void UDocumentComponent::PlayAmbientSound()
{
    if (!ReadingAmbientSound) return;
    
    // Stop existing ambient sound
    StopAmbientSound();
    
    AmbientAudioComponent = UGameplayStatics::SpawnSound2D(
        this, 
        ReadingAmbientSound, 
        0.5f,  // Volume
        1.0f,  // Pitch
        0.0f,  // Start time
        nullptr, 
        false, // Don't persist
        true   // Auto destroy
    );
}

void UDocumentComponent::StopAmbientSound()
{
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        if (AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->FadeOut(0.5f, 0.0f);
        }
        AmbientAudioComponent = nullptr;
    }
}

// ============================================================================
// HORROR GAME SPECIFIC
// ============================================================================

void UDocumentComponent::ApplySanityLossOnRead(float SanityLoss)
{
    if (SanityLoss <= 0.0f) return;

    // TODO: Integrate with your Sanity System
    // Example:
    // if (USanitySubsystem* SanitySystem = GetWorld()->GetSubsystem<USanitySubsystem>())
    // {
    //     SanitySystem->ReduceSanity(SanityLoss);
    // }

    UE_LOG(LogTemp, Warning, TEXT("DocumentComponent: Applied sanity loss: %.2f"), SanityLoss);
}

void UDocumentComponent::TriggerHorrorEvent(FName EventID)
{
    if (EventID.IsNone()) return;

    // TODO: Integrate with Horror Event System
    // Examples:
    // - Spawn enemy
    // - Trigger jumpscare
    // - Change ambient lighting
    // - Play horror sounds
    
    UE_LOG(LogTemp, Warning, TEXT("DocumentComponent: Triggered horror event: %s"), *EventID.ToString());
}

void UDocumentComponent::ScheduleHorrorEvent()
{
    if (HorrorEventID.IsNone()) return;
    
    ClearHorrorEventTimer();
    
    GetWorld()->GetTimerManager().SetTimer(
        HorrorEventTimerHandle,
        this,
        &UDocumentComponent::ExecuteDelayedHorrorEvent,
        HorrorEventDelay,
        false
    );
}

void UDocumentComponent::ExecuteDelayedHorrorEvent()
{
    TriggerHorrorEvent(HorrorEventID);
}

void UDocumentComponent::ClearHorrorEventTimer()
{
    if (HorrorEventTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(HorrorEventTimerHandle);
        HorrorEventTimerHandle.Invalidate();
    }
}

// ============================================================================
// INPUT MODE MANAGEMENT
// ============================================================================

void UDocumentComponent::SetInputModeUI()
{
    if (!CachedPlayerController || !CurrentDocumentWidget) return;
    
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(CurrentDocumentWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    
    CachedPlayerController->SetInputMode(InputMode);
    CachedPlayerController->bShowMouseCursor = true;
}

void UDocumentComponent::SetInputModeGame()
{
    if (!CachedPlayerController) return;
    
    FInputModeGameOnly InputMode;
    CachedPlayerController->SetInputMode(InputMode);
    CachedPlayerController->bShowMouseCursor = false;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

bool UDocumentComponent::LoadDocumentData(FName ItemID, UDataTable* DataTable)
{
    if (!DataTable) return false;

    FItemData* ItemRow = DataTable->FindRow<FItemData>(ItemID, TEXT("DocumentComponent"));
    if (!ItemRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("DocumentComponent: Item %s not found in DataTable"), *ItemID.ToString());
        return false;
    }

    CurrentDocumentData = *ItemRow;
    return true;
}

void UDocumentComponent::ResetDocumentState()
{
    bIsDocumentOpen = false;
    CurrentDocumentID = NAME_None;
    CurrentReadingTime = 0.0f;
    bIsTrackingReadTime = false;
    SetComponentTickEnabled(false);
}

void UDocumentComponent::StartReadingTimeTracking()
{
    CurrentReadingTime = 0.0f;
    bIsTrackingReadTime = true;
}

void UDocumentComponent::StopReadingTimeTracking()
{
    bIsTrackingReadTime = false;
    
    // TODO: Log analytics
    // FAnalytics::RecordEvent("DocumentRead", {
    //     {"DocumentID", CurrentDocumentID.ToString()},
    //     {"ReadTime", CurrentReadingTime}
    // });
}

void UDocumentComponent::CleanupResources()
{
    // Close any open document
    if (bIsDocumentOpen)
    {
        // Force close without animations
        if (CurrentDocumentWidget && IsValid(CurrentDocumentWidget))
        {
            CurrentDocumentWidget->RemoveFromParent();
            CurrentDocumentWidget = nullptr;
        }
        
        StopAmbientSound();
        SetInputModeGame();
    }
    
    // Clear all timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
    
    // Reset state
    ResetDocumentState();
    CachedPlayerController = nullptr;
}

UDocumentWidget* UDocumentComponent::GetCurrentDocumentWidget() const
{
    return CurrentDocumentWidget;
}

bool UDocumentComponent::IsDocumentOpen() const
{
    return bIsDocumentOpen;
}

FName UDocumentComponent::GetCurrentDocumentID() const
{
    return CurrentDocumentID;
}

float UDocumentComponent::GetCurrentReadingTime() const
{
    return CurrentReadingTime;
}