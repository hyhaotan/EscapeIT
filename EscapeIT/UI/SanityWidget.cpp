// Fill out your copyright notice in the Description page of Project Settings.

#include "SanityWidget.h"
#include "EscapeIT//Actor/Components/SanityComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USanityWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SanityProgress)
	{
		SanityProgress->SetPercent(1.0f);
	}

	if (SanityText)
	{
		SanityText->SetText(FText::FromString(TEXT("100 / 100")));
	}
}

void USanityWidget::InitializeSanityWidget(USanityComponent* InSanityComponent)
{
	if (!InSanityComponent)
	{
		return;
	}

	SanityComponent = InSanityComponent;

	SanityComponent->OnSanityChanged.AddDynamic(this, &USanityWidget::OnSanityChanged);

	UpdateSanityDisplay(SanityComponent->GetSanity());
}

void USanityWidget::UpdateSanityDisplay(float CurrentSanity)
{
	if (!SanityComponent)
	{
		return;
	}

	float MaxSanity = SanityComponent->GetMaxSanity();
	float SanityPercent = SanityComponent->GetSanityPercent();

	if (SanityProgress)
	{
		SanityProgress->SetPercent(SanityPercent);
	}

	if (SanityText)
	{
		FString SanityString = FString::Printf(TEXT("%.0f / %.0f"), CurrentSanity, MaxSanity);
		SanityText->SetText(FText::FromString(SanityString));
	}
}

void USanityWidget::OnSanityChanged(float NewSanity)
{
	UpdateSanityDisplay(NewSanity);
}