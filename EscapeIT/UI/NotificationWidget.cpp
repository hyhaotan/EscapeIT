// Fill out your copyright notice in the Description page of Project Settings.


#include "NotificationWidget.h"

#include "Components/TextBlock.h"

void UNotificationWidget::ShowSuccess(const FText& SuccessText)
{
    if (NotificationText)
    {
        NotificationText->SetText(SuccessText);
    }
}

void UNotificationWidget::ShowError(const FText& ErrorText)
{
    if (NotificationText)
    {
        NotificationText->SetText(ErrorText);
    }
}
