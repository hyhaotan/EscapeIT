
#include "SelectionSettingRow.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h" // your selection widget

USelectionSettingRow::USelectionSettingRow(const FObjectInitializer& Obj)
    : Super(Obj)
    , CurrentIndex(0)
    , bUpdating(false)
{
}

void USelectionSettingRow::NativeConstruct()
{
    Super::NativeConstruct();

    if (SelectionControl)
    {
        // Ensure the control is clean
        SelectionControl->Clear();
        // Bind to its selection changed delegate
        SelectionControl->OnSelectionChanged.AddDynamic(this, &USelectionSettingRow::HandleSelectionChanged);
    }
}

void USelectionSettingRow::NativeDestruct()
{
    if (SelectionControl)
    {
        SelectionControl->OnSelectionChanged.RemoveDynamic(this, &USelectionSettingRow::HandleSelectionChanged);
    }

    Super::NativeDestruct();
}

void USelectionSettingRow::InitializeRow(const TArray<FText>& Options, int32 InitialIndex, const FText& InLabel)
{
    if (!SelectionControl) return;

    bUpdating = true;
    SelectionControl->Clear();

    for (const FText& Opt : Options)
    {
        // FSelectionOption is assumed in your project; adapt if different
        SelectionControl->AddOption(FSelectionOption{ Opt });
    }

    CurrentIndex = FMath::Clamp(InitialIndex, 0, SelectionControl->GetOptionCount() - 1);
    SelectionControl->SetCurrentSelection(CurrentIndex);

    if (!InLabel.IsEmpty() && LabelText)
    {
        LabelText->SetText(InLabel);
    }

    bUpdating = false;
}

void USelectionSettingRow::SetOptions(const TArray<FText>& Options)
{
    if (!SelectionControl) return;
    bUpdating = true;
    SelectionControl->Clear();
    for (const FText& Opt : Options)
    {
        SelectionControl->AddOption(FSelectionOption{ Opt });
    }
    CurrentIndex = 0;
    SelectionControl->SetCurrentSelection(CurrentIndex);
    bUpdating = false;
}

void USelectionSettingRow::AddOption(const FText& OptionLabel)
{
    if (!SelectionControl) return;
    SelectionControl->AddOption(FSelectionOption{ OptionLabel });
}

void USelectionSettingRow::ClearOptions()
{
    if (!SelectionControl) return;
    SelectionControl->Clear();
    CurrentIndex = 0;
}

void USelectionSettingRow::SetCurrentSelection(int32 Index, bool bTriggerDelegate)
{
    if (!SelectionControl) return;

    const int32 Clamped = FMath::Clamp(Index, 0, SelectionControl->GetOptionCount() - 1);
    if (Clamped == CurrentIndex) return;

    bUpdating = true;
    CurrentIndex = Clamped;
    SelectionControl->SetCurrentSelection(CurrentIndex);
    bUpdating = false;

    if (bTriggerDelegate)
    {
        OnSelectionChanged.Broadcast(CurrentIndex);
    }
}

void USelectionSettingRow::SetLabel(const FText& InLabel)
{
    if (LabelText)
    {
        LabelText->SetText(InLabel);
    }
}

void USelectionSettingRow::RequestFocus()
{
    if (SelectionControl && GetOwningPlayer())
    {
        SelectionControl->SetUserFocus(GetOwningPlayer());
    }
}

void USelectionSettingRow::HandleSelectionChanged(int32 NewIndex)
{
    if (bUpdating) return;

    CurrentIndex = NewIndex;
    OnSelectionChanged.Broadcast(NewIndex);
}

