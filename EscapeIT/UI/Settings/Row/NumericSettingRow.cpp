#include "NumericSettingRow.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Kismet/KismetTextLibrary.h"

UNumericSettingRow::UNumericSettingRow(const FObjectInitializer& Obj)
    : Super(Obj)
    , MinValue(0.0f)
    , MaxValue(1.0f)
    , Step(0.01f)
    , CurrentValue(0.0f)
    , bUpdating(false)
{
}

void UNumericSettingRow::NativeConstruct()
{
    Super::NativeConstruct();

    if (ValueSlider)
    {
        ValueSlider->SetMinValue(MinValue);
        ValueSlider->SetMaxValue(MaxValue);
        // Step is not directly supported by USlider; you can snap on change
        ValueSlider->OnValueChanged.AddDynamic(this, &UNumericSettingRow::HandleSliderChanged);
    }

    if (ValueEditable)
    {
        ValueEditable->SetIsReadOnly(false);
        ValueEditable->SetIsEnabled(true);
        ValueEditable->OnTextCommitted.AddDynamic(this, &UNumericSettingRow::HandleTextCommitted);
        ValueEditable->OnTextChanged.AddDynamic(this, &UNumericSettingRow::HandleTextChanged);
    }

    // Initialize display
    SetValue(CurrentValue, false);
}

void UNumericSettingRow::NativeDestruct()
{
    if (ValueSlider)
    {
        ValueSlider->OnValueChanged.RemoveDynamic(this, &UNumericSettingRow::HandleSliderChanged);
    }
    if (ValueEditable)
    {
        ValueEditable->OnTextCommitted.RemoveDynamic(this, &UNumericSettingRow::HandleTextCommitted);
        ValueEditable->OnTextChanged.RemoveDynamic(this, &UNumericSettingRow::HandleTextChanged);
    }

    Super::NativeDestruct();
}

void UNumericSettingRow::InitializeRow(float InMin, float InMax, float InStep, float InInitialValue, const FText& InLabel)
{
    MinValue = InMin;
    MaxValue = InMax;
    Step = InStep;

    if (ValueSlider)
    {
        ValueSlider->SetMinValue(MinValue);
        ValueSlider->SetMaxValue(MaxValue);
    }

    CurrentValue = FMath::Clamp(InInitialValue, MinValue, MaxValue);

    if (!InLabel.IsEmpty())
    {
        SetLabel(InLabel);
    }

    SetValue(CurrentValue, false);
}

void UNumericSettingRow::SetValue(float NewValue, bool bTriggerDelegate)
{
    const float Snapped = (Step > 0.0f) ? FMath::RoundToFloat(NewValue / Step) * Step : NewValue;
    const float Clamped = FMath::Clamp(Snapped, MinValue, MaxValue);

    if (FMath::IsNearlyEqual(Clamped, CurrentValue, KINDA_SMALL_NUMBER))
        return;

    bUpdating = true;
    CurrentValue = Clamped;

    if (ValueSlider)
    {
        ValueSlider->SetValue(Clamped);
    }
    if (ValueEditable)
    {
        ValueEditable->SetText(FText::FromString(FString::SanitizeFloat(Clamped)));
    }
    bUpdating = false;

    if (bTriggerDelegate)
    {
        OnNumericValueChanged.Broadcast(CurrentValue);
    }
}

void UNumericSettingRow::SetLabel(const FText& InLabel)
{
    if (LabelText)
    {
        LabelText->SetText(InLabel);
    }
}

void UNumericSettingRow::RequestEditFocus()
{
    if (ValueEditable && GetOwningPlayer())
    {
        ValueEditable->SetUserFocus(GetOwningPlayer());
    }
}

void UNumericSettingRow::HandleSliderChanged(float Value)
{
    if (bUpdating) return;

    // Snap to step
    float Snapped = (Step > 0.0f) ? FMath::RoundToFloat(Value / Step) * Step : Value;
    Snapped = FMath::Clamp(Snapped, MinValue, MaxValue);

    bUpdating = true;
    CurrentValue = Snapped;
    if (ValueEditable)
    {
        ValueEditable->SetText(FText::FromString(FString::SanitizeFloat(Snapped)));
    }
    bUpdating = false;

    OnNumericValueChanged.Broadcast(CurrentValue);
}

void UNumericSettingRow::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    if (bUpdating) return;

    const FString S = Text.ToString().TrimStartAndEnd();
    if (S.IsEmpty())
    {
        // reset display
        if (ValueEditable)
            ValueEditable->SetText(FText::FromString(FString::SanitizeFloat(CurrentValue)));
        return;
    }

    // parse safely
    const float Parsed = FCString::Atof(*S);
    const float Snapped = (Step > 0.0f) ? FMath::RoundToFloat(Parsed / Step) * Step : Parsed;
    const float Clamped = FMath::Clamp(Snapped, MinValue, MaxValue);

    bUpdating = true;
    CurrentValue = Clamped;
    if (ValueSlider)
        ValueSlider->SetValue(Clamped);
    if (ValueEditable)
        ValueEditable->SetText(FText::FromString(FString::SanitizeFloat(Clamped)));
    bUpdating = false;

    OnNumericValueChanged.Broadcast(CurrentValue);
}

void UNumericSettingRow::HandleTextChanged(const FText& Text)
{
    if (bUpdating) return;

    // Live preview: only update if the text is parseable
    const FString S = Text.ToString().TrimStartAndEnd();
    if (S.IsEmpty()) return;

    // allow early rejection of non-numeric input
    bool bHasDigit = false;
    int32 Dot = 0;
    for (TCHAR C : S)
    {
        if (FChar::IsDigit(C)) bHasDigit = true;
        else if (C == '.') Dot++;
        else if (C == '-') continue;
        else { return; } // non numeric => skip live update
    }
    if (!bHasDigit) return;

    const float Parsed = FCString::Atof(*S);
    const float Snapped = (Step > 0.0f) ? FMath::RoundToFloat(Parsed / Step) * Step : Parsed;
    const float Clamped = FMath::Clamp(Snapped, MinValue, MaxValue);

    bUpdating = true;
    if (ValueSlider)
        ValueSlider->SetValue(Clamped);
    bUpdating = false;

    // update internal model but don't broadcast (avoid noisy events)
    CurrentValue = Clamped;
}
