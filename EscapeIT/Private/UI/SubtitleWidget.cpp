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
		NameText->SetText(FText::FromString(TEXT("")));
	}

	if (SubtitleText)
	{
		SubtitleText->SetText(FText::FromString(TEXT("")));
	}
}

void USubtitleWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoHideTimerHandle);
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
	ShowSubtitleAnimWidget();
	SetVisibility(ESlateVisibility::Visible);

	if (SubtitleBorder)
	{
		SubtitleBorder->SetVisibility(ESlateVisibility::Visible);
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

void USubtitleWidget::DisplaySubtitle(const FText& Name, const FText& Subtitle,USoundBase* Voice, float Duration)
{
	FSubtitleLine NewSubtitle;
	NewSubtitle.Name = Name;
	NewSubtitle.Subtitle = Subtitle;
	NewSubtitle.Voice = Voice;
	NewSubtitle.Duration = Duration;
	NewSubtitle.bKeepPreviousName = false;
	
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
		ProcessNextSubtitle();
	}
}

void USubtitleWidget::ProcessNextSubtitle()
{
	FSubtitleLine NextSubtitle;
	if (SubtitleQueue.Dequeue(NextSubtitle))
	{
		bIsDisplayingSubtitle = true;

		// Nếu bKeepPreviousName = true, giữ nguyên tên cũ
		if (!NextSubtitle.bKeepPreviousName || CurrentName.IsEmpty())
		{
			CurrentName = NextSubtitle.Name;
			SetNameText(NextSubtitle.Name);
		}
		// Nếu không, chỉ cập nhật subtitle
        
		SetSubtitleText(NextSubtitle.Subtitle);
		ShowSubtitle();

		// Phát âm thanh nếu có
		if (NextSubtitle.Voice && GetWorld())
		{
			UGameplayStatics::PlaySound2D(
				GetWorld()->GetFirstPlayerController()->GetPawn(), 
				NextSubtitle.Voice
			);
		}

		// Set timer để tự động ẩn
		if (NextSubtitle.Duration > 0.0f && GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(AutoHideTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				AutoHideTimerHandle,
				this,
				&USubtitleWidget::OnAutoHideTimer,
				NextSubtitle.Duration,
				false
			);
		}
	}
	else
	{
		bIsDisplayingSubtitle = false;
		CurrentName = FText::GetEmpty();
	}
}

void USubtitleWidget::StopSubtitleSequence()
{
	// Xóa hết queue
	FSubtitleLine Temp;
	while (SubtitleQueue.Dequeue(Temp)) {}
    
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoHideTimerHandle);
	}
    
	HideSubtitle();
	ClearSubtitles();
	bIsDisplayingSubtitle = false;
	CurrentName = FText::GetEmpty();
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

void USubtitleWidget::SkipCurrentSubtitle()
{
}

void USubtitleWidget::PauseSubtitleSequence()
{
}

void USubtitleWidget::ResumeSubtitleSequence()
{
}

int32 USubtitleWidget::GetRemainingSubtitleCount() const
{
}

void USubtitleWidget::DisplaySubtitleInternal(const FSubtitleLine& Line)
{
}

void USubtitleWidget::OnAutoHideTimer()
{
	HideSubtitle();
	ClearSubtitles();

	bIsDisplayingSubtitle = false;
    
	// Tiếp tục hiển thị subtitle tiếp theo trong queue
	ProcessNextSubtitle();
}

