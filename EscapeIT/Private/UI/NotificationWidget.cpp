// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/NotificationWidget.h"
#include "Components/TextBlock.h"

void UNotificationWidget::ShowSuccess(const FText& SuccessText)
{
    if (NotificationText)
    {
        NotificationText->SetText(SuccessText);
        ShowAnimWidget();
    }
    
    SetDelayNotification(3.0f);
}

void UNotificationWidget::ShowError(const FText& ErrorText)
{
    if (NotificationText)
    {
        NotificationText->SetText(ErrorText);
        ShowAnimWidget();
    }
    
    SetDelayNotification(3.0f);
}

void UNotificationWidget::ShowNotification(const FText& Text)
{
    AddToViewport(999);
    if (NotificationText)
    {
        NotificationText->SetText(Text);
        ShowAnimWidget();
    }
    SetDelayNotification(3.0f);
}

float UNotificationWidget::SetDelayNotification(float Delay)
{
    GetWorld()->GetTimerManager().SetTimer(TimerHandle,[this]()
    {
        HideAnimWidget();
    },Delay,false);
    return Delay;
}
