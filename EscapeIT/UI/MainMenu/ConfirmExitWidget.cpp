// Fill out your copyright notice in the Description page of Project Settings.


#include "ConfirmExitWidget.h"

#include "Components/Button.h"

void UConfirmExitWidget::NativeConstruct()
{
    Super::NativeConstruct();
    ConfirmExitButton->OnClicked.AddDynamic(this, &UConfirmExitWidget::OnConfirmExitClicked);
    CancelExitButton->OnClicked.AddDynamic(this, &UConfirmExitWidget::OnCancelExitClicked);
}

void UConfirmExitWidget::OnConfirmExitClicked()
{
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetWindow()->RequestDestroyWindow();
    }

    this->RemoveFromViewport();
}

void UConfirmExitWidget::OnCancelExitClicked()
{
    this->HideAnimExit();
    this->SetVisibility(ESlateVisibility::Collapsed);
}