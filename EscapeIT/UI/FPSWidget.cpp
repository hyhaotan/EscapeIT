// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FPSWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"

UFPSWidget::UFPSWidget(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
}

void UFPSWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (FPSText)
	{
		GetWorld()->GetTimerManager().SetTimer(
			FPSTimerHandle,
			this,
			&UFPSWidget::UpdateFPSCounter,
			UpdateRate,
			true);
	}
}

void UFPSWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (FPSTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FPSTimerHandle);
		FPSTimerHandle.Invalidate();
	}
}

void UFPSWidget::UpdateFPSCounter()
{
    if (!FPSText || !GEngine)
        return;

    float CurrentFPS = GetCurrentFPS();

    if (FPSHistory.Num() > 0 || MinFPS < 999.0f)
    {
        UpdateFPSHistory(CurrentFPS);
    }
    
    FString FPSString;
    FLinearColor TextColor = FLinearColor::White;
    
    if (CurrentFPS >= 60.0f)
    {
        FPSString = FString::Printf(TEXT("FPS: %.0f"), CurrentFPS);
        TextColor = FLinearColor(0.0f, 1.0f, 0.0f);
    }
    else if (CurrentFPS >= 30.0f)
    {
        FPSString = FString::Printf(TEXT("FPS: %.0f"), CurrentFPS);
        TextColor = FLinearColor(1.0f, 1.0f, 0.0f); 
    }
    else
    {
        FPSString = FString::Printf(TEXT("FPS: %.0f (Low)"), CurrentFPS);
        TextColor = FLinearColor(1.0f, 0.0f, 0.0f); 
    }
    

    
    // Frame time
    float FrameTimeMS = CurrentFPS > 0.0f ? (1000.0f / CurrentFPS) : 0.0f;
    FPSString += FString::Printf(TEXT(" (%.1fms)"), FrameTimeMS);

    // Stats (nếu bật)
    if (bShowDetailedStats && FPSHistory.Num() > 30)
    {
        float AvgFPS = GetAverageFPS();
        FPSString += FString::Printf(TEXT("\nAvg: %.0f | Min: %.0f | Max: %.0f"), 
            AvgFPS, MinFPS, MaxFPS);
    }
    
    FPSText->SetText(FText::FromString(FPSString));
    FPSText->SetColorAndOpacity(TextColor);
}

float UFPSWidget::GetCurrentFPS() const
{
    extern ENGINE_API float GAverageFPS;
    if (GAverageFPS > 0.0f)
    {
        return GAverageFPS;
    }

    if (GetWorld())
    {
        float DeltaTime = GetWorld()->GetDeltaSeconds();
        if (DeltaTime > 0.0f)
        {
            return 1.0f / DeltaTime;
        }
    }
    return 1.0f;
}

void UFPSWidget::UpdateFPSHistory(float CurrentFPS)
{
    CurrentFPS = FMath::Clamp(CurrentFPS, 0.0f, 999.0f);
    
    FPSHistory.Add(CurrentFPS);

    if (FPSHistory.Num() > MaxFPSHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }

    if (CurrentFPS < MinFPS) MinFPS = CurrentFPS;
    if (CurrentFPS > MaxFPS) MaxFPS = CurrentFPS;
}

void UFPSWidget::EnableAdvancedFPSMonitoring(bool bEnable)
{
    if (bEnable)
    {
        FPSHistory.Empty();
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
    }
    else
    {
        FPSHistory.Empty();
    }
}

float UFPSWidget::GetAverageFPS() const
{
    if (FPSHistory.Num() == 0) return 0.0f;
    
    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }
    
    return Sum / FPSHistory.Num();
}

float UFPSWidget::GetMaxFPS() const
{
    return MaxFPS;
}

float UFPSWidget::GetMinFPS() const
{
    return MinFPS < 999.0f ? MinFPS : 0.0f;
}

