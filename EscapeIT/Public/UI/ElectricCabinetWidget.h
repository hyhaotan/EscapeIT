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

class UTextBlock;
class UButton;
class UUniformGridPanel;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleTimedOut);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateRepairedPuzzleTime,float, RepairTime);

UCLASS()
class ESCAPEIT_API UElectricCabinetWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    // Widget components
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* GridPanel;
    
    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* InstructionText;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ProgressText;
    
    UPROPERTY(meta=(BindWidget))
    UTextBlock* RepairTimeText;
    
    UPROPERTY(meta=(BindWidgetAnim),Transient)
    UWidgetAnimation* ShowAnim;
     
    UPROPERTY(meta=(BindWidgetAnim),Transient)
    UWidgetAnimation* HideAnim;
    
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation")
    UAnimMontage* ElectricShockAnim;
    
    // Puzzle settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    int32 GridWidth = 9;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    int32 GridHeight = 9;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    int32 NumberOfWirePairs = 3;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    float MinimumPointDistance = 1.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    float RepairDuration = 30.0f;
    
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Puzzle Events")
    FOnPuzzleCompleted OnPuzzleCompleted;
    
    UPROPERTY(BlueprintAssignable, Category = "Puzzle Events")
    FOnPuzzleTimedOut OnPuzzleTimedOut; 
    
    UPROPERTY(BlueprintAssignable, Category = "Puzzle Events")
    FOnUpdateRepairedPuzzleTime OnUpdateRepairedPuzzleTime;
    
    UFUNCTION()
    void ShowAnimWidget(){PlayAnimation(ShowAnim);}   
    
    UFUNCTION()
    void HideAnimWidget(){PlayAnimation(HideAnim);}
    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
private:
    UPROPERTY()
    TArray<FGridCell> GridCells;
    
    UPROPERTY()
    TMap<FIntPoint, int32> PositionToIndexMap;
    
    UPROPERTY()
    TArray<class UBorder*> CellBorders;
    
    UPROPERTY()
    TArray<FWirePair> WirePairs;
    
    bool bIsDragging;
    EWireColor CurrentDragColor;
    int32 CurrentPathID;
    FIntPoint LastCellPosition;
    
    float RemainingTime;
    bool bTimerActive;
    bool bPuzzleFinished;
    
    void InitializePuzzle();
    void GenerateRandomPuzzle();
    void CreateGridUI();
    
    bool IsPuzzleSolvable(const TArray<FWirePair>& WiresToCheck);
    bool HasPathBetweenPoints(FIntPoint Start, FIntPoint End, const TArray<FIntPoint>& BlockedPositions);
    
    bool IsPositionValid(FIntPoint Position, const TArray<FIntPoint>& UsedPositions);
    float GetDistanceBetweenPoints(FIntPoint A, FIntPoint B);
    
    void UpdateCellVisual(int32 CellIndex);
    FLinearColor GetColorForWireType(EWireColor WireColor);
    
    void StartDrag(FIntPoint CellPosition);
    void UpdateDrag(FIntPoint CellPosition);
    void EndDrag();
    
    bool CanPlacePath(FIntPoint Position, EWireColor Color);
    void ClearPath(EWireColor Color);
    void AutoClearCurrentPath();
    bool IsCellBlockedByOtherPath(FIntPoint Position, EWireColor Color);
    
    bool CheckPuzzleComplete();
    void UpdateProgressDisplay();
    
    FGridCell* GetCellAt(FIntPoint Position);
    int32 GetCellIndex(FIntPoint Position);
    FIntPoint ScreenToGridPosition(const FGeometry& Geometry, const FVector2D& ScreenPosition);
    FString GetColorName(EWireColor Color);
    
    bool GenerateCompletePath(FWirePair& OutWire);
    bool GeneratePathBetweenPoints(FIntPoint Start, FIntPoint End, TArray<FIntPoint>& OutPath);
    FIntPoint ChooseWeightedRandom(const TArray<FIntPoint>& Choices, const TArray<float>& Weights);
    FIntPoint FindRandomEmptyCell();
    void MarkPathOnGrid(const TArray<FIntPoint>& Path, EWireColor Color);
    void ResetGridToEmpty();
    void ConvertSolutionToPuzzle(TArray<FWirePair>& Wires);
    
    void UpdateRepairedPuzzleTime();
    void OnRepairedPuzzleCountDown(float DeltaTime); 
    void OnTimerExpired();
    
    UFUNCTION()
    void OnTimerExpiredAnimationFinished();
    
    void PauseTimer();
    void ResumeTimer();
    void ResetTimer();
    
    UFUNCTION()
    void OnCloseButtonClicked();
};