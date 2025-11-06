// SelectionSettingRow.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SelectionSettingRow.generated.h"

class UTextBlock;
class UImage;
class USelectionWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionSettingChanged, int32, NewIndex);

UCLASS()
class ESCAPEIT_API USelectionSettingRow : public UUserWidget
{
    GENERATED_BODY()

public:
    USelectionSettingRow(const FObjectInitializer& Obj);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Initialize the row with options and initial selection */
    UFUNCTION(BlueprintCallable, Category = "SelectionRow")
    void InitializeRow(const TArray<FText>& Options, int32 InitialIndex = 0, const FText& InLabel = FText::GetEmpty());

    UFUNCTION(BlueprintCallable, Category = "SelectionRow")
    void SetOptions(const TArray<FText>& Options);

    UFUNCTION(BlueprintCallable, Category = "SelectionRow")
    void AddOption(const FText& OptionLabel);

    UFUNCTION(BlueprintCallable, Category = "SelectionRow")
    void ClearOptions();

    UFUNCTION(BlueprintCallable, Category = "SelectionRow")
    void SetCurrentSelection(int32 Index, bool bTriggerDelegate = true);

    UFUNCTION(BlueprintCallable, Category = "SelectionRow")
    int32 GetCurrentSelection() const { return CurrentIndex; }

    UFUNCTION(BlueprintCallable, Category = "SelectionRow")
    void SetLabel(const FText& InLabel);

    /** Request keyboard focus for the selection control (if it supports focus) */
    UFUNCTION(BlueprintCallable, Category = "SelectionRow")
    void RequestFocus();

    /** Delegate broadcast when selection changes (index) */
    UPROPERTY(BlueprintAssignable, Category = "SelectionRow")
    FOnSelectionSettingChanged OnSelectionChanged;

protected:
    // Widgets (bind in UMG Blueprint)
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* LabelText;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionWidget* SelectionControl;

    // Internal
    int32 CurrentIndex = 0;
    bool bUpdating = false;

    // Internal callback from the selection widget
    UFUNCTION()
    void HandleSelectionChanged(int32 NewIndex);
};