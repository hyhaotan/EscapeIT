// ElectricCabinetWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/FlowPuzzleTypes.h"
#include "Components/UniformGridPanel.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ElectricCabinetWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleCompleted);

UCLASS()
class ESCAPEIT_API UElectricCabinetWidget : public UUserWidget
{
    GENERATED_BODY()
    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
public:
    UPROPERTY(BlueprintAssignable, Category = "Puzzle")
    FOnPuzzleCompleted OnPuzzleCompleted;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    int32 GridWidth = 6;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    int32 GridHeight = 6;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    float CellSize = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings", meta = (ClampMin = "1", ClampMax = "6"))
    int32 NumberOfWirePairs = 3;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings", meta = (ClampMin = "2"))
    int32 MinimumPointDistance = 2;
    
    UPROPERTY()
    TArray<FWirePair> WirePairs;
    
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* GridPanel;
    
    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;
    
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* InstructionText;
    
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* ProgressText;
    
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void InitializePuzzle();
    
    UFUNCTION(BlueprintPure, Category = "Puzzle")
    FLinearColor GetColorForWireType(EWireColor WireColor);
    
private:
    TArray<FGridCell> GridCells;
    TMap<FIntPoint, int32> PositionToIndexMap;
    
    bool bIsDragging;
    EWireColor CurrentDragColor;
    int32 CurrentPathID;
    FIntPoint LastCellPosition;
    
    TArray<UBorder*> CellBorders;
    
    void GenerateRandomPuzzle();
    bool IsPositionValid(FIntPoint Position, const TArray<FIntPoint>& UsedPositions);
    float GetDistanceBetweenPoints(FIntPoint A, FIntPoint B);
    FString GetColorName(EWireColor Color);
    
    void CreateGridUI();
    
    void UpdateCellVisual(int32 CellIndex);
    
    void StartDrag(FIntPoint CellPosition);
    void UpdateDrag(FIntPoint CellPosition);
    void EndDrag();
    
    bool CanPlacePath(FIntPoint Position, EWireColor Color);
    
    void AutoClearCurrentPath();
    
    void ClearPath(EWireColor Color);
    
    bool CheckPuzzleComplete();
    
    void UpdateProgressDisplay();
    
    FGridCell* GetCellAt(FIntPoint Position);
    int32 GetCellIndex(FIntPoint Position);
    
    FIntPoint ScreenToGridPosition(const FGeometry& Geometry, const FVector2D& ScreenPosition);
    
    bool IsCellBlockedByOtherPath(FIntPoint Position, EWireColor Color);
    
    UFUNCTION()
    void OnCloseButtonClicked();
};