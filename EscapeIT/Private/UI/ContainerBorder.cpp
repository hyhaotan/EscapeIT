// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ContainerBorder.h"

void UContainerBorder::OnNormalBorder()
{
	SetBrushColor(NormalBorder);
}

void UContainerBorder::OnHoverBorder()
{
	SetBrushColor(HoverBorder);
}
