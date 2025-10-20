#include "SelectionWidget.h"
#include "CommonTextBlock.h"
#include "Logging/StructuredLog.h"

USelectionWidget::USelectionWidget()
{
    CurrentSelection = 0;
    SetIsFocusable(true);
    SetVisibilityInternal(ESlateVisibility::Visible);
}

void USelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Only setup navigation if we have options
    if (Options.Num() > 0)
    {
        UpdateCurrentSelection();

        FCustomWidgetNavigationDelegate NavigationDelegate;
        NavigationDelegate.BindDynamic(this, &USelectionWidget::OnNavigation);
        SetNavigationRuleCustom(EUINavigation::Left, NavigationDelegate);
        SetNavigationRuleCustom(EUINavigation::Right, NavigationDelegate);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No options set for selection widget"));
    }
}

void USelectionWidget::Clear()
{
    Options.Reset();
    CurrentSelection = 0;
}

void USelectionWidget::AddOption(const FSelectionOption& InOption)
{
    Options.Add(InOption);

    // If this is the first option being added, update the display
    if (Options.Num() == 1)
    {
        CurrentSelection = 0;
        UpdateCurrentSelection();
    }
}

void USelectionWidget::SetCurrentSelection(int InIndex)
{
    if (InIndex < 0 || InIndex >= Options.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("SetCurrentSelection: Index %d is out of bounds (Options count: %d)"), InIndex, Options.Num());
        return;
    }

    CurrentSelection = InIndex;
    UpdateCurrentSelection();
}

void USelectionWidget::SelectPrevious()
{
    OnNavigation(EUINavigation::Left);
}

void USelectionWidget::SelectNext()
{
    OnNavigation(EUINavigation::Right);
}

UWidget* USelectionWidget::OnNavigation(EUINavigation InNavigation)
{
    if (Options.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnNavigation: No options available"));
        return this;
    }

    if (InNavigation != EUINavigation::Left && InNavigation != EUINavigation::Right)
    {
        return this;
    }

    const auto Direction = InNavigation == EUINavigation::Left ? -1 : 1;
    CurrentSelection += Direction;

    if (CurrentSelection < 0)
    {
        CurrentSelection = Options.Num() - 1;
    }
    else if (CurrentSelection >= Options.Num())
    {
        CurrentSelection = 0;
    }

    UpdateCurrentSelection();
    OnSelectionChange.ExecuteIfBound(CurrentSelection);
    return this;
}

void USelectionWidget::UpdateCurrentSelection()
{
    // Safety checks
    if (!Label)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateCurrentSelection: Label widget is null"));
        return;
    }

    if (Options.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateCurrentSelection: No options available"));
        Label->SetText(FText::FromString(TEXT("No Options")));
        return;
    }

    if (CurrentSelection < 0 || CurrentSelection >= Options.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateCurrentSelection: CurrentSelection %d is out of bounds (Options count: %d)"), CurrentSelection, Options.Num());
        CurrentSelection = FMath::Clamp(CurrentSelection, 0, Options.Num() - 1);
    }

    Label->SetText(Options[CurrentSelection].Label);
}
