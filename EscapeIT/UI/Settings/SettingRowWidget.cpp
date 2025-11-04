// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingRowWidget.h"

#include "EscapeIT/UI/ContainerBorder.h"

void USettingRowWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (ContainerBorder)
    {
        ContainerBorder->OnHoverBorder();
    }
}

void USettingRowWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    if (ContainerBorder)
    {
        ContainerBorder->OnNormalBorder();
    }
}