// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SubtitleWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void USubtitleWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (NameText)
    {
        NameText->SetText(FText::GetEmpty());
    }

    if (SubtitleText)
    {
        SubtitleText->SetText(FText::GetEmpty());
    }
}

void USubtitleWidget::NativeDestruct()
{
    Super::NativeDestruct();

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
        World->GetTimerManager().ClearTimer(DelayTimerHandle);
    }
}

void USubtitleWidget::SetNameText(const FText& Text)
{
    if (NameText)
    {
        NameText->SetText(Text);
        
        OnUpdateNameText.Broadcast(Text);
        OnNameTextChanged.Broadcast();

        if (Text.IsEmpty())
        {
            NameText->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            NameText->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void USubtitleWidget::SetSubtitleText(const FText& Text)
{
    if (SubtitleText)
    {
        SubtitleText->SetText(Text);
        
        OnUpdateSubtitleText.Broadcast(Text);
        OnSubtitleTextChanged.Broadcast();

        if (!Text.IsEmpty())
        {
            ShowSubtitle();
        }
    }
}

void USubtitleWidget::ShowSubtitle()
{
    if (GetVisibility() == ESlateVisibility::Collapsed)
    {
        SetVisibility(ESlateVisibility::Visible);
        AddToViewport(100);

        if (SubtitleBorder)
        {
            SubtitleBorder->SetVisibility(ESlateVisibility::Visible);
        }
        
        ShowSubtitleAnimWidget();
    }
    else
    {
        DisplaySubtitleTextAnimWidget();
    }
}

void USubtitleWidget::HideSubtitle()
{
    SetVisibility(ESlateVisibility::Collapsed);

    if (SubtitleBorder)
    {
        SubtitleBorder->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    HideSubtitleAnimWidget();
}

void USubtitleWidget::HideSubtitleTextOnly()
{
    HideSubtitleTextAnimWidget();
}

bool USubtitleWidget::HasMoreSubtitlesInQueue() const
{
    return !SubtitleQueue.IsEmpty();
}

void USubtitleWidget::DisplaySubtitle(const FText& Name, const FText& Subtitle, USoundBase* Voice, float Duration)
{
    FSubtitleLine NewSubtitle = UDialogueDataHelpers::MakeSubtitleLine(Name,Subtitle,Voice,Duration,false,1.0f,true);
    
    SubtitleQueue.Enqueue(NewSubtitle);

    if (!bIsDisplayingSubtitle)
    {
        ProcessNextSubtitle();
    }
}

void USubtitleWidget::DisplaySubtitleSequence(const TArray<FSubtitleLine>& Subtitles)
{
    for (const FSubtitleLine& Line : Subtitles)
    {
        SubtitleQueue.Enqueue(Line);
    }

    if (!bIsDisplayingSubtitle)
    {
        CurrentLineIndex = 0;
        ProcessNextSubtitle();
    }
}

void USubtitleWidget::ProcessNextSubtitle()
{
    if (bIsPaused)
    {
        return;
    }

    FSubtitleLine NextSubtitle;
    if (SubtitleQueue.Dequeue(NextSubtitle))
    {
        CurrentSubtitle = NextSubtitle;
        
        if (NextSubtitle.DelayBeforeShow > 0.0f)
        {
            UWorld* World = GetWorld();
            if (World)
            {
                World->GetTimerManager().SetTimer(
                    DelayTimerHandle,
                    [this]()
                    {
                        DisplaySubtitleInternal(CurrentSubtitle);
                    },
                    NextSubtitle.DelayBeforeShow,
                    false
                );
            }
        }
        else
        {
            DisplaySubtitleInternal(NextSubtitle);
        }
    }
    else
    {
        bIsDisplayingSubtitle = false;
        CurrentName = FText::GetEmpty();
        CurrentLineIndex = 0;
        
        HideSubtitle();
        OnSubtitleSequenceComplete.Broadcast();
    }
}

void USubtitleWidget::DisplaySubtitleInternal(const FSubtitleLine& Line)
{
    bIsDisplayingSubtitle = true;

    if (!Line.bKeepPreviousName || CurrentName.IsEmpty())
    {
        CurrentName = Line.Name;
        SetNameText(Line.Name);
    }
    
    SetSubtitleText(Line.Subtitle);
    ShowSubtitle();

    UWorld* World = GetWorld();
    if (Line.Voice && World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC)
        {
            APawn* Pawn = PC->GetPawn();
            if (Pawn)
            {
                UGameplayStatics::PlaySound2D(Pawn, Line.Voice);
            }
            else
            {

                UGameplayStatics::PlaySound2D(PC, Line.Voice);
            }
        }
    }

    if (Line.Duration > 0.0f && World)
    {
        World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
        World->GetTimerManager().SetTimer(
            AutoHideTimerHandle,
            this,
            &USubtitleWidget::OnAutoHideTimer,
            Line.Duration,
            false
        );
    }
}

void USubtitleWidget::StopSubtitleSequence()
{
    FSubtitleLine Temp;
    while (SubtitleQueue.Dequeue(Temp)) {}
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
        World->GetTimerManager().ClearTimer(DelayTimerHandle);
    }
    
    HideSubtitle();
    ClearSubtitles();
    bIsDisplayingSubtitle = false;
    bIsPaused = false;
    CurrentName = FText::GetEmpty();
    CurrentLineIndex = 0;
}

void USubtitleWidget::ClearSubtitles()
{
    if (NameText)
    {
        NameText->SetText(FText::GetEmpty());
        NameText->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (SubtitleText)
    {
        SubtitleText->SetText(FText::GetEmpty());
    }
}

void USubtitleWidget::OnAutoHideTimer()
{
    OnSubtitleLineComplete.Broadcast(CurrentLineIndex);
    CurrentLineIndex++;
    
    bool bHasMoreSubtitles = HasMoreSubtitlesInQueue();

    if (bHasMoreSubtitles)
    {
        HideSubtitleTextOnly();

        if (SubtitleText)
        {
            SubtitleText->SetText(FText::GetEmpty());
        }
    }
    else
    {
        HideSubtitle();
        ClearSubtitles();
    }

    bIsDisplayingSubtitle = false;
    
    ProcessNextSubtitle();
}

void USubtitleWidget::SkipCurrentSubtitle()
{
    if (!bIsDisplayingSubtitle || !CurrentSubtitle.bCanSkip)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
        World->GetTimerManager().ClearTimer(DelayTimerHandle);
    }

    OnAutoHideTimer();
}

void USubtitleWidget::PauseSubtitleSequence()
{
    if (!bIsDisplayingSubtitle || bIsPaused)
    {
        return;
    }

    bIsPaused = true;
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().PauseTimer(AutoHideTimerHandle);
        World->GetTimerManager().PauseTimer(DelayTimerHandle);
    }
}

void USubtitleWidget::ResumeSubtitleSequence()
{
    if (!bIsPaused)
    {
        return;
    }

    bIsPaused = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().UnPauseTimer(AutoHideTimerHandle);
        World->GetTimerManager().UnPauseTimer(DelayTimerHandle);
    }
}

int32 USubtitleWidget::GetRemainingSubtitleCount()
{
    int32 Count = 0;
    TArray<FSubtitleLine> TempArray;
    FSubtitleLine Temp;

    while (SubtitleQueue.Dequeue(Temp))
    {
        TempArray.Add(Temp);
        Count++;
    }

    // Restore the queue
    for (const FSubtitleLine& Line : TempArray)
    {
        SubtitleQueue.Enqueue(Line);
    }

    return Count;
}
