#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NumericSettingRow.generated.h"

class UTextBlock;
class USlider;
class UEditableTextBox;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNumericSettingChanged, float, NewValue);

/**
 * Reusable numeric setting row: Label + Slider + EditableTextBox
 * - BindWidgetOptional so you can create a UMG Widget Blueprint inheriting from this.
 * - Exposes Initialize / SetValue / GetValue + delegate OnNumericValueChanged
 */
UCLASS()
class ESCAPEIT_API UNumericSettingRow : public UUserWidget
{
    GENERATED_BODY()

public:
    UNumericSettingRow(const FObjectInitializer& Obj);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Initialize row (can also be set from editor via properties) */
    UFUNCTION(BlueprintCallable, Category = "SettingRow")
    void InitializeRow(float InMin, float InMax, float InStep, float InInitialValue, const FText& InLabel);

    UFUNCTION(BlueprintCallable, Category = "SettingRow")
    void SetValue(float NewValue, bool bTriggerDelegate = true);

    UFUNCTION(BlueprintCallable, Category = "SettingRow")
    float GetValue() const { return CurrentValue; }

    UFUNCTION(BlueprintCallable, Category = "SettingRow")
    void SetLabel(const FText& InLabel);

    /** Request keyboard focus for the editable text box */
    UFUNCTION(BlueprintCallable, Category = "SettingRow")
    void RequestEditFocus();

    /** Delegate for consumers to bind to */
    UPROPERTY(BlueprintAssignable, Category = "SettingRow")
    FOnNumericSettingChanged OnNumericValueChanged;

protected:
    // Widgets (bind in UMG)
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* LabelText;

    UPROPERTY(meta = (BindWidgetOptional))
    USlider* ValueSlider;

    UPROPERTY(meta = (BindWidgetOptional))
    UEditableTextBox* ValueEditable;

    // optional decorative icon
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* IconImage;

    // Limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SettingRow")
    float MinValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SettingRow")
    float MaxValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SettingRow")
    float Step = 0.01f;

    // Internal
    float CurrentValue = 0.0f;
    bool bUpdating = false;

    // Callbacks
    UFUNCTION()
    void HandleSliderChanged(float Value);

    UFUNCTION()
    void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitType);

    UFUNCTION()
    void HandleTextChanged(const FText& Text);
};
